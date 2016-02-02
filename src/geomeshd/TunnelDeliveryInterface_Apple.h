//
// Created by System Administrator on 1/30/16.
//

#ifndef GEOMESH_TUNNELINTERFACE_H
#define GEOMESH_TUNNELINTERFACE_H

#include <memory>
#include <sys/kern_control.h>
#include "../LocalInterface.h"

// No need for aome #IF APPLE since this include file is already only used on Apple
#define TUNInterface_IFNAMSIZ 16

class TunnelDeliveryInterface_Apple {

    LocalInterface *mLocalInterface;
    struct sockaddr_ctl addr;

    char mReceptionBuffer[MAX_PACKET_SIZE - IPv6_START];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int mSocketId;

    Address iFaceAddress;

    void assignIP();

public:

    TunnelDeliveryInterface_Apple(LocalInterface *localInterface, const Address &iFaceAddress);

    void startTunnelInterface();

    void deliverIPv6Packet(DataBufferPtr packet);

    void pollMessages();


};


#endif //GEOMESH_TUNNELINTERFACE_APPLE_H
