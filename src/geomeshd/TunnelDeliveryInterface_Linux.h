//
// Created by System Administrator on 2/2/16.
//

#ifndef GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
#define GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H

#include "../LocalInterface.h"
#include "../PacketFunctions.h"
#include "../constants.h"

#define TUNInterface_IFNAMSIZ 16

class TunnelDeliveryInterface_Linux {

    LocalInterface *mLocalInterface;

    char mReceptionBuffer[MAX_PACKET_SIZE - IPv6_START];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int fd;

    Address iFaceAddress;

    void assignIP();

public:

    TunnelDeliveryInterface_Linux(LocalInterface *localInterface, const Address &iFaceAddress);

    void startTunnelInterface();

    void deliverIPv6Packet(DataBufferPtr packet);

    void pollMessages();


};


#endif //GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
