//
// Created by System Administrator on 2/2/16.
//

#ifndef GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
#define GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H


#define TUNInterface_IFNAMSIZ 16

class TunnelDeliveryInterface_Apple {

    LocalInterface *mLocalInterface;
    struct sockaddr_ctl addr;

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
