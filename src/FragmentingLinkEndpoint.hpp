//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UDPINTERFACE_H
#define GEOMESH_UDPINTERFACE_H

#include "AbstractLinkEndpoint.hpp"
#include "FragmentingLinkEndpoint.hpp"
#include "Logger.hpp"
#include <netinet/in.h>
#include <string>
#include <string.h>
#include "PacketDefragmenter.hpp"

/**
 * How many reception buffers each UDP interface has.
 * Having more will reduce packet loss in case multiple GeoMesh packets are fragmented,
 * and the fragments arrive in disorder.
 */
const int UDP_RECEPTION_BUFFER_COUNT = 2;

const int UDP_FRAGMENT_SIZE = 576; // IPv4 minimum MTU.

typedef std::function<void (const PacketFragmentPtr&, uint16_t localIfaceID)>
            FragmentHandlerCallback;

/**
 * A subclass of AbstractLinkEndpoint that exposes UDP links to the LinkManager.
 * This class doesn't do much on its own, and passes any data to be sent to
 * the UDPManager.
 */
class FragmentingLinkEndpoint : public AbstractLinkEndpoint {

    // The interface number of the corresponding FragmentingLinkEndpoint at the other end of the link.
    int mRemoteIface;

    uint16_t nextPacketNumber;

    time_t lastMessage;

    // The address of the peer at the other end of the link.
    struct sockaddr_in peerAddress;

    // An array of reception buffers.
    PacketDefragmenter receptionBuffers[UDP_RECEPTION_BUFFER_COUNT];

    FragmentHandlerCallback fragmentHandler;

public:

    /**
     * Simple constructor that records the reference to the UDPManager
     */
    FragmentingLinkEndpoint(FragmentHandlerCallback handler);

    /**
     * Set the address of the peer. This address is the one that is written as the destination
     * in the UDP packet headers.
     */
    void setPeerAddress(struct sockaddr_in remoteAddr) {

        Logger::log(LogLevel::DEBUG, "UDP bridge interface " 
                + std::to_string(iFaceID) + " peer address changed.");

        peerAddress = remoteAddr;

    }

    /**
     * What it says on the tin. Sends a GeoMesh packet over the UDP link.
     * Note: message may be fragmented (but reconstructed at the other end),
     * delivery not guaranteed over UDP.
     */
    bool sendData(PacketPtr data) override;

    /**
     * Called by the UDPManager to notify this interface of the arrival of 
     * a new fragment over the UDP bridge destined to this iterface.
     *
     * The FragmentingLinkEndpoint will insert the fragment in a reception 
     * buffer and try to reconstruct a packet.
     */
    void fragmentReceived(PacketFragmentPtr frag);

    /**
     * Called when a full GeoMesh packet is received from the wire side.
     * Not to be confused with UDP bridge fragment packets.
     */
    void packetReceived(PacketPtr data) {
        dataArrivedCallback(data, iFaceID);
    }

    /**
     * Get the interface number of the FragmentingLinkEndpoint at the other side.
     */
    int getMRemoteIface() const {
        return mRemoteIface;
    }

    /**
     * Set the remote interface.
     */
    void setMRemoteIface(int mRemoteIface) {
        FragmentingLinkEndpoint::mRemoteIface = mRemoteIface;
    }

    /**
     * Used to distinguish different packets after fragmenting them. Please note that these numbers
     * are NOT unique, but should be unique enough to know which packets are being sent across.
     */
    uint16_t getNextPacketNumber() {
        return ++nextPacketNumber;
    }

};


#endif //GEOMESH_UDPINTERFACE_H
