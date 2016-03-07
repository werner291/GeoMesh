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
#include "UDPReceptionBuffer.h"

class UDPManager;

/**
 * How many reception buffers each UDP interface has.
 * Having more will reduce packet loss in case multiple GeoMesh packets are fragmented,
 * and the fragments arrive in disorder.
 */
const int UDP_RECEPTION_BUFFER_COUNT = 2;

const int UDP_FRAGMENT_SIZE = 576; // IPv4 minimum MTU.

/**
 * A subclass of AbstractInterface that exposes UDP links to the LinkManager.
 * This class doesn't do much on its own, and passes any data to be sent to
 * the UDPManager.
 */
class UDPInterface : public AbstractInterface {

    // UDPInterface is basially an extension of UDPManager that allows the UDPManager
    // to be inserted as one or more interfaces into the LinkManager.
    friend UDPManager;

private:
    // The interface number of the corresponding UDPInterface at the other end of the link.
    int mRemoteIface;

    int nextPacketNumber;

    // The address of the peer at the other end of the link.
    struct sockaddr_in peerAddress;

    // An array of reception buffers.
    UDPReceptionBuffer receptionBuffers[UDP_RECEPTION_BUFFER_COUNT];

    // A reference to the UDPManager corresponding to this interface.
    UDPManager* udpMan;

    /**
     * To be called by UDPManager. A fragment of data has arrived, store it in
     * the reception buffer. Process the resulting packet if all have arrived.
     * Please note that packets may be lost or reordered over UDP.
     */
    void dataFragmentReceived() {

    }

public:

    /**
     * Simple constructor that records the reference to the UDPManager
     */
    UDPInterface(UDPManager* udpMan);

    /**
     * Set the address of the peer. This address is the one that is written as the destination
     * in the UDP packet headers.
     */
    void setPeerAddress(struct sockaddr_in remoteAddr) {

        Logger::log(LogLevel::DEBUG, "UDP bridge interface " + std::to_string(iFaceID) + " peer address changed.");

        peerAddress = remoteAddr;

    }

    /**
     * What it says on the tin. Sends a GeoMesh packet over the UDP link.
     * Note: message may be fragmented (but reconstructed at the other end),
     * delivery not guaranteed over UDP.
     */
    bool sendData(PacketPtr data) override;

    void fragmentReceived(UDPFragmentPtr frag) {
        UDPReceptionBuffer& reception = receptionBuffers[frag->getPacketNumber() % UDP_RECEPTION_BUFFER_COUNT];

        reception.receive(frag);

        if (reception.isFullPacketAvailable()) {
            Logger::log(DEBUG, "Reconstructed packet " + std::to_string(frag->getPacketNumber()));
            packetReceived(reception.getReconstructedPacket());
        }
    }

    /**
     * Called when a full GeoMesh packet is received from the wire side.
     * Not to be confused with UDP link packets.
     */
    void packetReceived(PacketPtr data) {
        this->dataArrivedCallback(data, iFaceID);
    }

    /**
     * Get the interface number of the UDPInterface at the other side.
     */
    int getMRemoteIface() const {
        return mRemoteIface;
    }

    /**
     * Set the remote interface.
     */
    void setMRemoteIface(int mRemoteIface) {
        UDPInterface::mRemoteIface = mRemoteIface;
    }

    uint16_t getNextPacketNumber() {
        return ++nextPacketNumber;
    }

};


#endif //GEOMESH_UDPINTERFACE_H
