//
// Created by System Administrator on 2/1/16.
//

#ifndef GEOMESH_UDPCONNECTIONMANAGER_H
#define GEOMESH_UDPCONNECTIONMANAGER_H

#include "../LinkManager.h"
#include "UDPInterface.h"
#include <string>
#include <string.h>

/**
 * Starts, maintains and stops UDP bridge links to other direct peers.
 */
class UDPManager {

    friend UDPInterface;

    LinkManager* linkMgr;

    int socketID;

    int localPort;

    std::map<uint16_t, std::shared_ptr<UDPInterface> > connectingLinks;

    std::map<uint16_t, std::shared_ptr<UDPInterface> > establishedLinks;


public:
    UDPManager(LinkManager *linkMgr);

    /**
     * Send a UDP bridge hello packet to the specified address and port,
     * and creates and connects an interface associated to that peer.
     */
    void connectTo(std::string address, int port);

    /**
     * Polls the socket for new packets, and sends them to the router if there are any.
     */
    void pollMessages();

    void processBridgeControlMessage(char *buffer, sockaddr_in &sender);

    bool sendMessage(PacketPtr message, UDPInterface* iFace) {

        uint8_t sendBuffer[message->getDataLength() + 2];

        ((uint16_t*)sendBuffer)[0] = htons(iFace->mRemoteIface);

        memcpy(sendBuffer + 2, message->getData(), message->getDataLength());

        int result = sendto(socketID,
                            sendBuffer,
                            message->getDataLength() + 2,
                            0,
                            (struct sockaddr *) &iFace->peerAddress,
                            sizeof(iFace->peerAddress));

        if (result < 0) {
            Logger::log(LogLevel::ERROR, "UDPInterface: error while sending: " + std::string(strerror(errno)));
        }

        return result == 0;
    }

    void processNormalPacket(const uint8_t *buffer, int nbytes, uint16_t localIface);
};


#endif //GEOMESH_UDPCONNECTIONMANAGER_H
