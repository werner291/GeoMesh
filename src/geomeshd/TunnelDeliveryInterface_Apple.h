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

/**
 * The class responsible for creating and managing the utun device used to deliver or accept
 * IPv6 datagrams to and from Mac OSX systems.
 */
class TunnelDeliveryInterface_Apple {

    // Reference to the LocalInterface of the Router.
    LocalInterface *mLocalInterface;

    // The address of the AF_SYSTEM socket associated with the utun device.
    struct sockaddr_ctl addr;

    uint8_t mReceptionBuffer[MAX_PACKET_SIZE - GEOMESH_PAYLOAD_START];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int mSocketId;

    Address iFaceAddress;

    /**
     * Assign the GeoMesh pseudo-IPv6 address to the interface.
     */
    void assignIP();

    /**
     * Add an entry in the host's routing table to redirect all traffic found for fcf4:/16 addresses
     * to the utun device.
     */
    void installRoute();

public:

    TunnelDeliveryInterface_Apple(LocalInterface *localInterface, const Address &iFaceAddress);

    /**
     * As the name implies, allocate a new utun interface and set it up so the system can
     * use GeoMesh as if it was any other IPv6 network.
     */
    void startTunnelInterface();

    /**
     * To be called by the Router's LocalInterface. The GeoMesh packet will be unwrapped
     * and delivered to the host system through the utun device.
     */
    void deliverIPv6Packet(PacketPtr packet);

    /**
     * Poll the utun interface for new messages.
     */
    void pollMessages();


};


#endif //GEOMESH_TUNNELINTERFACE_APPLE_H
