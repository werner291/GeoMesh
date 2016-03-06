//
// Created by System Administrator on 2/1/16.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include "UDPManager.h"

UDPManager::UDPManager(LinkManager *linkMgr, int localPort) : linkMgr(linkMgr) {

    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    localPort = localPort;
    sin.sin_port = htons(localPort);

    if (bind(socketID, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        Logger::log(LogLevel::ERROR, "Error while binding UDP bridge control socket: " + std::string(strerror(errno)));
    } else {
        Logger::log(LogLevel::INFO, "UDP bridge listening on port " + std::to_string(ntohs(sin.sin_port)));
    }

    // Enable non-blocking IO.
    int flags = fcntl(socketID, F_GETFL, 0);
    fcntl(socketID, F_SETFL, flags | O_NONBLOCK);
}

void UDPManager::connectTo(std::string address, int port) {

    // Allocate the interface
    std::shared_ptr<UDPInterface> iface(new UDPInterface(this));
    connectingLinks.insert(std::make_pair(iface->getInterfaceId(), iface));

    struct sockaddr_in destAddr;

    // Send the hello message
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = inet_addr(address.c_str());

    iface->setPeerAddress(destAddr);

    std::stringstream helloMsg;

    helloMsg << "GeoMesh_UDP_Bridge_Hello cientIfaceID:" << iface->getInterfaceId();

    std::string msg = helloMsg.str();

    uint8_t buffer[500];

    ((uint16_t *) buffer)[0] = htons(0);

    strncpy(reinterpret_cast<char *>(buffer + 2), msg.c_str(), msg.length());

    buffer[2 + msg.length()] = 0;

    sendto(socketID,
           buffer,
           500,
           0,
           (struct sockaddr *) &destAddr,
           sizeof(destAddr));

    Logger::log(LogLevel::INFO, "Peering request sent to " + address);

}

void UDPManager::pollMessages() {

    uint8_t buffer[MAX_PACKET_SIZE + 2];

    sockaddr_in sender;
    int len = sizeof(sender);

    int nbytes = recvfrom(socketID, buffer, MAX_PACKET_SIZE + 2, 0, (struct sockaddr *) &sender, (socklen_t *) &len);

    if (nbytes > 0) { // It's a proper datagaram (not an error)

        uint16_t localIface = ntohs(((uint16_t *) buffer)[0]);

        Logger::log(LogLevel::DEBUG, "Received UDP datagram for iFace " + std::to_string(localIface));

        if (localIface == 0) {
            // This is a message directed at the UDPManager
            processBridgeControlMessage((char *) buffer + 2, sender);
        } else {
            // This is a message directed at one of the UDPInterfaces
            processNormalPacketFragment(buffer + 2, nbytes - 2, localIface);
        }
    } else if (nbytes == 0) {
        // Received empty packet?
        Logger::log(LogLevel::DEBUG, "Received empty packet?!");
    } else {
        int err = errno;

        if (!(err == EWOULDBLOCK || err == EAGAIN)) {
            Logger::log(LogLevel::ERROR,
                        "UDPManager: receive error: " + std::string(strerror(err)));
        }
        // Else there was nothing to be read, do nothing
    }
}

void UDPManager::processNormalPacketFragment(const uint8_t *buffer, int nbytes, uint16_t localIface) {
    UDPFragmentPtr fragment(new UDPFragment(buffer, nbytes, true));

    auto itr = establishedLinks.find(localIface);

    if (itr == establishedLinks.end()) {
        Logger::log(WARN, "Received message for interface " + std::to_string(localIface)
                          + " but this is not a known or established UDP interface.");
    } else {
        itr->second->fragmentReceived(fragment);
    }
}

void UDPManager::processBridgeControlMessage(char *buffer, sockaddr_in &sender) {
    if (strncmp(buffer, "GeoMesh_UDP_Bridge_Hello", strlen("GeoMesh_UDP_Bridge_Hello")) == 0) {

        // Extract the remote interface id and port number
        int clientIfaceID;
        sscanf(buffer, "GeoMesh_UDP_Bridge_Hello cientIfaceID:%i", &clientIfaceID);

        // Store the information about our new peer, including the address and GeoMesh port (NOT the bridge control port)
        std::shared_ptr<UDPInterface> newIface(new UDPInterface(this));
        newIface->setPeerAddress(sender);
        newIface->setMRemoteIface(clientIfaceID);

        establishedLinks.insert(std::make_pair(newIface->getInterfaceId(), newIface));

        ((uint16_t *) buffer)[0] = htons(0);

        // Generate a response message containing the remote interface id (NOT THE LOCAL ONE!),
        // as well as the LOCAL GeoMesh port
        // Note that these are swapped!
        sprintf(buffer+2, "GeoMesh_UDP_Bridge_Established cientIfaceID:%i serverIfaceID:%i", clientIfaceID, newIface->getInterfaceId());

        // Send it back to the remote (use the bridge control port, which is the port from which the hello was sent)
        sendto(socketID,
               buffer,
               strlen(buffer+2)+2,
               0,
               (struct sockaddr *) &sender, // Return to sender
               sizeof(sender));

        usleep(1000);

        // Tell the router about the new link. (AFTER SENDING ESTABLISHED!!! or the receiver will ignore the packet
        // and we'll have to wait for the scheduled re-send (TODO implement resending)
        linkMgr->connectInterface(newIface);

        Logger::log(INFO, "Received peering request, remote interface ID is " +
                          std::to_string(newIface->getMRemoteIface()));

        // This is a response to a hello message we sent previously
    } else if (strncmp(buffer, "GeoMesh_UDP_Bridge_Established", strlen("GeoMesh_UDP_Bridge_Established")) == 0) {

        // Extract the local interface id and port number
        // The interface id allows us to identify for which local interface we sent the message.
        int ifaceID, remoteIfaceID;
        sscanf(buffer, "GeoMesh_UDP_Bridge_Established cientIfaceID:%i serverIfaceID:%i", &ifaceID, &remoteIfaceID);

        auto itr = connectingLinks.find(ifaceID);
        if (itr != connectingLinks.end()) {

            establishedLinks.insert(std::make_pair(itr->first, itr->second));

            itr->second->setMRemoteIface(remoteIfaceID);

            linkMgr->connectInterface(itr->second);

            connectingLinks.erase(itr);
        } else {
            Logger::log(WARN, "Received unexpected GeoMesh_UDP_Bridge_Established: " + std::string(buffer));
        }

        Logger::log(INFO, "Peering request confirmed.");
    } else {
        Logger::log(WARN, "Received invalid message: " + std::string(buffer));
    }
}

bool UDPManager::sendMessage(PacketPtr message, UDPInterface* iFace) {

    Logger::log(LogLevel::ERROR, "UDPManager: sending UDP message to remote iFace: "
                                 + std::to_string(iFace->mRemoteIface));

    std::vector<UDPFragmentPtr> fragments = fragmentPacket(message, iFace->getNextPacketNumber(), iFace->mRemoteIface);

    for (UDPFragmentPtr frag : fragments) {

        int result = sendto(socketID,
                            frag->getPayloadData(),
                            frag->getDataLength(),
                            0,
                            (struct sockaddr *) &iFace->peerAddress,
                            sizeof(iFace->peerAddress));

        if (result < 0) {
            Logger::log(LogLevel::ERROR, "UDPManager: error while sending: " + std::string(strerror(errno)));
            return false;
        }
    }



    return true;
}

std::vector<UDPFragmentPtr> UDPManager::fragmentPacket(const PacketPtr &message, uint16_t packetNum, uint16_t remoteIface) {

    std::vector<UDPFragmentPtr> fragments;

    int maxFragSize = 500;

    for (int fragStart = 0; fragStart < message->getDataLength(); fragStart += maxFragSize) {

        int fragLength = std::min(maxFragSize, message->getDataLength() - fragStart);

        UDPFragmentPtr frag(new UDPFragment(message->getData()+fragStart, fragLength, false));

        frag->setPacketNumber(packetNum);

        frag->setPacketLength(message->getDataLength());

        frag->setDestinationInterfaceID(remoteIface);

        frag->setFragmentStart(fragStart);

        frag->setFragmentLength(fragLength);

        fragments.push_back(frag);
    }
    return fragments;
}