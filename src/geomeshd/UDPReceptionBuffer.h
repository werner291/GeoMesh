//
// Created by System Administrator on 2/29/16.
//

#ifndef GEOMESH_UDPRECEPTIONBUFFER_H
#define GEOMESH_UDPRECEPTIONBUFFER_H

#include "UDPFragment.h"

#include "../Packet.h"

/**
 * The UDPReceptionBuffer helps to combine a number of UDPFragment objects into a packet.
 */
class UDPReceptionBuffer {

    // The packet number that corresponds to the packet that the last-received
    // fragment was part of.
    int packetNumber;

    // The size of the packet currently being receivd
    int expectedSize;

    // Whether the buffer currently stores a full packet.
    bool valid;

    /*
     * A list of byte ranges that have so far been received
     * If a pair (a,b) is in receivedRanges, this means that all bytes between
     * positions a (inclusive) and b (exclusive)
     */
    std::vector< std::pair<int,int> > receivedRanges;

    // The actual data
    std::vector<uint8_t> buffer;

    // Internal function that removes all ranges that are overlapping in receivedRanges
    void removeOverlappingRanges();

    /**
     * Mark a range of bytes as received.
     *
     * @param begin The first byte position (inclusive)
     * @param end The last byte position (exclusive)
     */
    void markReceived(int begin, int end);

public:

    /*
     * Create a new empty reception buffer.
     */
    UDPReceptionBuffer() : valid(false), expectedSize(0), packetNumber(0) {}

    /**
     * Receive and store a fragment of data.
     */
    bool receive(UDPFragmentPtr frag);

    /**
     * @return Whether the full packet has been received.
     */
    bool isFullPacketAvailable();

    /**
     * Returns the reconstructed packet.
     *
     * Call isFullPacketAvailable() first to check whether a packet can be reconstructed,
     * otherwise this call cause an error.
     */
    PacketPtr getReconstructedPacket();
};


#endif //GEOMESH_UDPRECEPTIONBUFFER_H
