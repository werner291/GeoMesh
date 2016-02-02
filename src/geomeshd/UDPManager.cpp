//
// Created by System Administrator on 2/1/16.
//

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sstream>
#include "UDPManager.h"
#include "../Logger.h"

UDPManager::UDPManager(LinkManager *linkMgr) : linkMgr(linkMgr) {
    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Enable non-blocking IO.
    int flags = fcntl(socketID, F_GETFL, 0);
    fcntl(socketID, F_SETFL, flags | O_NONBLOCK);
}

void UDPManager::connectTo(std::string address, int port) {

    // Allocate the interface
    std::shared_ptr<UDPInterface> iface(new UDPInterface());
    connectingLinks.push_back(iface);

    struct sockaddr_in destAddr;

    // Send the hello message
    destAddr.sin_family = AF_INET;
    destAddr.sin_port = htons(port);
    destAddr.sin_addr.s_addr = inet_addr(address.c_str());

    std::stringstream helloMsg;

    helloMsg << "GeoMesh_UDP_Bridge_Hello ifaceID:" << iface->getInterfaceId() << " UDP_port:" << iface->getLocalPort();

    std::string msg = helloMsg.str();

    sendto(socketID,
           msg.c_str(),
           msg.length(),
           0,
           (struct sockaddr *) &destAddr,
           sizeof(destAddr));

}

void UDPManager::pollMessages() {

    char buffer[500];

    sockaddr_in sender;
    int len = sizeof(sender);

    int nbytes = recvfrom(socketID, buffer, 499, 0, (struct sockaddr *) &sender, (socklen_t *) &len);


    buffer[499] = 0; // Set last to 0 for safety

    if (nbytes > 0) { // It's a proper datagaram (not an error)

        // This is a UDP bridge hello message
        if (strncmp(buffer, "GeoMesh_UDP_Bridge_Hello", strlen("GeoMesh_UDP_Bridge_Hello")) == 0) {

            // Extract the remote interface id and port number
            int ifaceID, port;
            sscanf(buffer, "GeoMesh_UDP_Bridge_Hello ifaceID:%i UDP_port:%i", &ifaceID, &port);

            // Store the information about our new peer, including the address and GeoMesh port (NOT the bridge control port)
            std::shared_ptr<UDPInterface> newIface(new UDPInterface());
            newIface->setPeerAddress(sender.sin_addr, port);

            // Tell the router about the new link.
            linkMgr->connectInterface(newIface);

            // Generate a response message containing the remote interface id (NOT THE LOCAL ONE!),
            // as well as the LOCAL GeoMesh port
            sprintf(buffer, "GeoMesh_UDP_Bridge_Established ifaceID:%i UDP_port:%i", ifaceID, newIface->getLocalPort());

            // Send it back to the remote (use the bridge control port, which is the port from which the hello was sent)
            sendto(socketID,
                   buffer,
                   strlen(buffer),
                   0,
                   (struct sockaddr *) &sender, // Return to sender
                   sizeof(sender));

            // This is a response to a hello message we sent previously
        } else if (strncmp(buffer, "GeoMesh_UDP_Bridge_Established", strlen("GeoMesh_UDP_Bridge_Hello")) == 0) {

            // Extract the local interface id and port number
            // The interface id allows us to identify for which local interface we sent the message.
            int ifaceID, port;
            sscanf(buffer, "GeoMesh_UDP_Bridge_Established ifaceID:%i UDP_port:%i", &ifaceID, &port);

            for (auto itr = connectingLinks.begin(); itr != connectingLinks.end(); itr++) {
                if ((*itr)->getInterfaceId() == ifaceID) {

                    (*itr)->setPeerAddress(sender.sin_addr, port);

                    establishedLinks.push_back(*itr);

                    linkMgr->connectInterface(*itr);

                    connectingLinks.erase(itr);
                    break;
                }
            }
        } else {
            Logger::log(LogLevel::WARN, "Received invalid message: " + std::string(buffer));
        }


    } else if (nbytes == 0) {


    } else {
        int err = errno;

        if (!(err == EWOULDBLOCK || err == EAGAIN)) {
            Logger::log(LogLevel::ERROR,
                        "UDPManager: receive error: " + std::string(strerror(err)));
        }
        // Elseo there was nothing to be read, do nothing
    }
}