/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GEOMESH_UDPRECEPTIONBUFFER_H
#define GEOMESH_UDPRECEPTIONBUFFER_H

#include "PacketFragment.hpp"

#include "Packet.hpp"

/**
 * The PacketDefragmenter helps to combine a number of PacketFragment objects into a packet.
 */
class PacketDefragmenter {

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
    PacketDefragmenter() : packetNumber(0), expectedSize(0), valid(false) {}

    /**
     * Receive and store a fragment of data.
     */
    bool receive(PacketFragmentPtr frag);

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
