//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UDPINTERFACE_H
#define GEOMESH_UDPINTERFACE_H

#include "../AbstractInterface.h"
#include "../constants.h"
#include "UDPInterface.h"
#include "../Logger.h"
#include <netinet/in.h>
#include <string>
#include <string.h>

class UDPInterface : public AbstractInterface {

    int socketID;

    int mLocalUDPport;

    struct sockaddr_in peerAddress;

    uint8_t mReceptionBuffer[MAX_PACKET_SIZE];

public:

    UDPInterface();

    void pollMessages();

    int getLocalPort() {
        return mLocalUDPport;
    }

    void setPeerAddress(const struct in_addr &addr, int remotePort) {

        Logger::log(LogLevel::DEBUG, "UDP bridge interface " + std::to_string(iFaceID) + " peer address changed.");

        peerAddress.sin_addr = addr;
        peerAddress.sin_port = remotePort;

    }

    bool sendData(PacketPtr data) override;

};


#endif //GEOMESH_UDPINTERFACE_H
