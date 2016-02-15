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

class UDPManager;

/**
 * A subclass of AbstractInterface that exposes UDP links to the LinkManager.
 * This class doesn't do much on its own, amd passes any data to be sent to
 * the UDPManager.
 */
class UDPInterface : public AbstractInterface {

    friend UDPManager;

private:
    int mRemoteIface;

    struct sockaddr_in peerAddress;

    uint8_t mReceptionBuffer[MAX_PACKET_SIZE];

    UDPManager* udpMan;

public:

    /**
     * Simple constructor that records the reference to the UDPManager
     */
    UDPInterface(UDPManager* udpMan);

    void setPeerAddress(struct sockaddr_in remoteAddr) {

        Logger::log(LogLevel::DEBUG, "UDP bridge interface " + std::to_string(iFaceID) + " peer address changed.");

        peerAddress = remoteAddr;

    }

    bool sendData(PacketPtr data) override;

    void packetReceived(PacketPtr data) {
        this->dataArrivedCallback(data, iFaceID);
    }

    int getMRemoteIface() const {
        return mRemoteIface;
    }

    void setMRemoteIface(int mRemoteIface) {
        UDPInterface::mRemoteIface = mRemoteIface;
    }

};


#endif //GEOMESH_UDPINTERFACE_H
