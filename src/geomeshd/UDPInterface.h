//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UDPINTERFACE_H
#define GEOMESH_UDPINTERFACE_H

#include "../AbstractInterface.h"
#include "../constants.h"
#include <netinet/in.h>

class UDPInterface : public AbstractInterface {

    int socketID;

    int mLocalUDPport;

    struct sockaddr_in peerAddress;

    char mReceptionBuffer[MAX_PACKET_SIZE];

public:

    UDPInterface();

    void pollMessages();

    int getLocalPort() {
        return mLocalUDPport;
    }

    void setPeerAddress(const struct in_addr &addr, int remotePort) {
        peerAddress.sin_addr = addr;
        peerAddress.sin_port = remotePort;

    }

    bool sendData(DataBufferPtr data) override;

};


#endif //GEOMESH_UDPINTERFACE_H
