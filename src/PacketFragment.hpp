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

#ifndef GEOMESH_UDPFRAGMENT_H
#define GEOMESH_UDPFRAGMENT_H

#include <vector>
#include <netinet/in.h>
#include "Packet.hpp"

/**
 * Represents a fragment of a packet to be sent over a fragmenting link.
 */
class PacketFragment {

    /**
     * A 12-byte header followed by payload information.
     *
     * Data structure pecification:
     *
     *      +-------------------+-------------------+
     *      | 0       | 1       | 2       | 3       |
     *      +-------------------+-------------------+
     *    0 | Protocol version  | Dest interface ID |
     *      +-------------------+-------------------+
     *    4 | Packet number     | Packet length     |
     *      +-------------------+-------------------+
     *    8 | Fragment start    | Fragment length   |
     *      +-------------------+-------------------+
     *   12 | Payload...                            |
     *      +---------------------------------------+
     *
     * @see "internet_bridge.md" for more details.
     *
     */
    std::vector<uint8_t> data;

    /**
     * Assign protocol version number to byte positions
     */
    inline void setProtocolVersion(uint16_t version) {
        *reinterpret_cast<uint16_t*>(data.data()) = htons(version);
    }

public:

    /**
     * @param includesHeader Whether the provided data includes the header, or whether space should
     *                       be reserved in the buffer to fit a header later on.
     */
    PacketFragment(const uint8_t* buffer, int bufferLength, bool includesHeader);

    inline int getProtocolVersion() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data()));
    }

    inline uint16_t getDestinationInterfaceID() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 2));
    }

    inline void setDestinationInterfaceID(uint16_t packetNum) {
        *reinterpret_cast<uint16_t*>(data.data() + 2) = htons(packetNum);
    }

    inline uint16_t getPacketNumber() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 4));
    }

    inline void setPacketNumber(uint16_t packetNum) {
        *reinterpret_cast<uint16_t*>(data.data() + 4) = htons(packetNum);
    }

    inline int getPacketLength() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 6));
    }

    inline void setPacketLength(int length) {
        assert(length <= MAX_PACKET_SIZE);
        *reinterpret_cast<uint16_t*>(data.data() + 6) = htons(length);
    }

    inline int getFragmentStart() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 8));
    }

    inline void setFragmentStart(uint16_t start) {
        *reinterpret_cast<uint16_t*>(data.data() + 8) = htons(start);
    }

    inline int getPayloadLength() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 10));
    }

    inline void setFragmentLength(uint16_t length) {
        *reinterpret_cast<uint16_t*>(data.data() + 10) = htons(length);
    }

    uint8_t*getPayloadData() {
        return data.data() + 12;
    }

    uint8_t* getDataBuffer() {
        return data.data();
    }

    size_t getDataLength() {
        return data.size();
    }


};

typedef std::shared_ptr<PacketFragment> PacketFragmentPtr;

#endif //GEOMESH_UDPFRAGMENT_H
