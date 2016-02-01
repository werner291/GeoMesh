//
// Created by System Administrator on 1/30/16.
//

#ifndef GEOMESH_TUNNELINTERFACE_H
#define GEOMESH_TUNNELINTERFACE_H

#include <memory>
#include "../LocalInterface.h"

// No need for aome #IF APPLE since this include file is already only used on Apple
#define TUNInterface_IFNAMSIZ 16

class TunnelInterface_Apple {

    std::weak_ptr<LocalInterface> mLocalInterface;

    char mReceptionBuffer[MAX_PACKET_SIZE - IPv6_START];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int mSocketId;

    TunnelInterface_Apple(std::weak_ptr<LocalInterface> localInterface);

    void startTunnelInterface();

    void deliverIPv6Packet(DataBufferPtr packet);

    void pollMessages();

    void assignIP(char *iFaceName, const Address& addr);
};


#endif //GEOMESH_TUNNELINTERFACE_APPLE_H
