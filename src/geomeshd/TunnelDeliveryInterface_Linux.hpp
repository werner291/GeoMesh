//
// Created by System Administrator on 2/2/16.
//

#ifndef GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
#define GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H

#include "../LocalInterface.hpp"
#include "../Packet.hpp"

#define TUNInterface_IFNAMSIZ 16

class TunnelDeliveryInterface_Linux {

    LocalInterface *mLocalInterface;

    uint8_t mReceptionBuffer[MAX_PAYLOAD_SIZE];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int fd;

    Address iFaceAddress;

    void assignIP();

    void installRoute();

public:

    TunnelDeliveryInterface_Linux(LocalInterface *localInterface, const Address &iFaceAddress);

    void startTunnelInterface();

    void deliverIPv6Packet(PacketPtr packet);

    void pollMessages();


};


#endif //GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H