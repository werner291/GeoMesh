/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>
#include <unistd.h>
#include <time.h>
#include "UDPManager.hpp"

UDPManager::UDPManager(LinkManager& linkMgr, int localPort,
        Scheduler& scheduler) : linkMgr(linkMgr) {

    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(localPort);

    if (bind(socketID, (struct sockaddr *) &sin, sizeof(sin)) < 0) {
        Logger::log(LogLevel::ERROR,
                "Error while binding UDP bridge control socket: " 
                + std::string(strerror(errno)));
    } else {
        Logger::log(LogLevel::INFO,
                "UDP bridge listening on port " 
                + std::to_string(ntohs(sin.sin_port)));
    }

    // Enable non-blocking IO.
    // TODO use some kind of centralized system that allows me to use select()
    // on UNIX-like systems
    int flags = fcntl(socketID, F_GETFL, 0);
    fcntl(socketID, F_SETFL, flags | O_NONBLOCK);

    scheduler.scheduleTask(Scheduler::Task(Scheduler::clock::now(),
                       std::chrono::milliseconds(10),
                       true,
                       std::bind(&UDPManager::pollMessages,this)));

}

void UDPManager::connectTo(std::string address, int port) {

    Logger::log(LogLevel::INFO,
            "Connecting to UDP peer at " + address + " port " 
            + std::to_string(port));

    struct sockaddr_in destAddr;

    // Send the hello message
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = inet_addr(address.c_str());
    
    // Allocate the interface
    std::shared_ptr<UnixSocketInterface> iface( new UnixSocketInterface(
                std::bind(&UDPManager::sendFragment, this, 
                    std::placeholders::_1, std::placeholders::_2), 
                (sockaddr_storage*)(&destAddr)));

    connectingLinks.insert(std::make_pair(iface->getInterfaceId(), iface));

    std::stringstream helloMsg;

    helloMsg << "GeoMesh_UDP_Bridge_Hello cientIfaceID:" << iface->getInterfaceId();

    std::string msg = helloMsg.str();

    sendControlMessage(msg,destAddr);

    Logger::log(LogLevel::INFO, "Peering request sent to " + address);

}

void UDPManager::sendControlMessage(const std::string& msg, struct sockaddr_in& destAddr) {
    uint8_t buffer[msg.length() + 1 + 4];

    ((uint16_t *) buffer)[0] = htons(0);
    ((uint16_t *) buffer)[1] = htons(0);
    
    strncpy(reinterpret_cast<char *>(buffer + 4), msg.c_str(), msg.length());

    buffer[4 + msg.length()] = 0;

    sendto(socketID,
           buffer,
           msg.length() + 4 + 1, // String length + 1 for \0 + 4 for header
           0,
           (struct sockaddr *) &destAddr,
           sizeof(destAddr));
}

void UDPManager::pollMessages() {

    uint8_t buffer[MAX_PACKET_SIZE + 4];

    sockaddr_in sender;
    int len = sizeof(sender);

    int nbytes = recvfrom(socketID, buffer, MAX_PACKET_SIZE + 4, 0, (struct sockaddr *) &sender, (socklen_t *) &len);

    if (nbytes > 0) { // It's a proper datagaram (not an error)

        uint16_t protVersion = ntohs(((uint16_t *) buffer)[0]);

        if (protVersion > 0) {
            Logger::log(LogLevel::WARN, "Unsupported protocol version on UDP"
                    " bridge! Consider upgrading your GeoMesh installation!");
            return;
        }

        uint16_t localIface = ntohs(((uint16_t *) buffer)[1]);

        if (localIface == 0) {
            // This is a message directed at the UDPManager
            processBridgeControlMessage((char *) buffer + 4, sender);
        } else {
            // This is a message directed at one of the FragmentingLinkEndpoints
            processNormalPacketFragment(buffer, nbytes, localIface);
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
    PacketFragmentPtr fragment(new PacketFragment(buffer, nbytes, true));

    auto itr = establishedLinks.find(localIface);

    if (itr == establishedLinks.end()) {
        Logger::log(WARN, "Received message for interface " + std::to_string(localIface)
                          + " but this is not a known or established UDP interface.");
    } else {
        itr->second->fragmentReceived(fragment);
    }
}

void UDPManager::processBridgeControlMessage(char *received,
                                             sockaddr_in &sender) {
    
    if (strncmp(received, "GeoMesh_UDP_Bridge_Hello",
                strlen("GeoMesh_UDP_Bridge_Hello")) == 0) {

        // Extract the remote interface id and port number
        int clientIfaceID;
        sscanf(received, "GeoMesh_UDP_Bridge_Hello cientIfaceID:%i",
                &clientIfaceID);

        // Store the information about our new peer, including the address and 
        // GeoMesh port (NOT the bridge control port)
        // Allocate the interface
        std::shared_ptr<UnixSocketInterface> newIface( new UnixSocketInterface(
                    std::bind(&UDPManager::sendFragment, this, 
                        std::placeholders::_1, std::placeholders::_2), 
                    (sockaddr_storage*)(&sender)));

        newIface->setMRemoteIface(clientIfaceID);

        establishedLinks.insert(std::make_pair(newIface->getInterfaceId(), newIface));

        std::stringstream response;
        response << "GeoMesh_UDP_Bridge_Established cientIfaceID:"
            << clientIfaceID << " serverIfaceID:"
            << newIface->getInterfaceId();

        sendControlMessage(response.str(), sender);

        usleep(1000);

        // Tell the router about the new link. (AFTER SENDING ESTABLISHED!!! or 
        // the receiver will ignore the packet and we'll have to wait for the
        // scheduled re-send (TODO implement resending)
        linkMgr.connectInterface(newIface);

        Logger::log(INFO, "Received peering request, remote interface ID is " +
                          std::to_string(newIface->getMRemoteIface()));

        // This is a response to a hello message we sent previously
    } else if (strncmp(received, "GeoMesh_UDP_Bridge_Established",
                strlen("GeoMesh_UDP_Bridge_Established")) == 0) {

        // Extract the local interface id and port number. The interface id
        // allows us to identify for which local interface we sent the message.
        int ifaceID, remoteIfaceID;
        sscanf(received, "GeoMesh_UDP_Bridge_Established cientIfaceID:%i"
               " serverIfaceID:%i", &ifaceID, &remoteIfaceID);

        // Find the interface associated with this ID (if any)
        auto itr = connectingLinks.find(ifaceID);
        if (itr != connectingLinks.end()) {

            // Yes, we were waiting for this. Transfer the interface
            // from the "waiting" to the "established"
            establishedLinks.insert(std::make_pair(itr->first, itr->second));

            itr->second->setMRemoteIface(remoteIfaceID);

            linkMgr.connectInterface(itr->second);

            connectingLinks.erase(itr);
        } else {
            // We didn't initiate a connection, so why are we getting
            // a confirmation?
            Logger::log(WARN,
                    "Received unexpected GeoMesh_UDP_Bridge_Established: " 
                    + std::string(received));
        }

        Logger::log(INFO, "Peering request confirmed.");
    } else {
        Logger::log(WARN, "Received invalid message: " + std::string(received));
    }
}

bool UDPManager::sendFragment(PacketFragmentPtr frag, uint16_t iFaceID) {

    auto itr = establishedLinks.find(iFaceID);

    assert(itr != establishedLinks.end());

    int result = sendto(socketID,
                        frag->getDataBuffer(),
                        frag->getDataLength(),
                        0,
                        (struct sockaddr *) &itr->second->addr,
                        sizeof(itr->second->addr));

    if (result < 0) {
        Logger::log(LogLevel::ERROR, "UDPManager: error while sending: " 
                + std::string(strerror(errno)));
        return false;
    }

    return true;
}

