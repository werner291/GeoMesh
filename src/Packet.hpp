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


#ifndef GEOMESH_PACKET_H
#define GEOMESH_PACKET_H

#include <stdint.h>
#include "Location.hpp"
#include "UniqueAddress.hpp"
#include <assert.h>
#include <limits.h>

// A few constants
const int PROTOCOL_VERSION = 0;
const int DEFAULT_TTL = 255; // IPv6 only has one byte for this, maybe GeoMesh should use its own?
const double FACE_ROUTING_RANGE_MULTIPLIER = 2;
const int MAX_PACKET_SIZE = 1500; // 1500 bytes

typedef double location_scalar;

// Different message types.
const uint8_t MSGTYPE_IPv6 = 0;          // A regular message containing actual data to be transmitted
const uint8_t MSGTYPE_LOCATION_INFO = 1;    // A message containing information about the sender and their direct peers

const uint8_t MSGTYPE_DHT_FIND_CLOSEST = 10;
const uint8_t MSGTYPE_DHT_FIND_RESPONSE = 11;
const uint8_t MSGTYPE_DHT_LEAVE = 12;

// Different routing modes
const int ROUTING_GREEDY = 0;
const int ROUTING_FACE_RIGHT = 1;
const int ROUTING_FACE_LEFT = 2;

// GeoMesh Version 0 packet header field positions
// Relative to header start.
const int GEOMESH_HEADER_PROTOCOL_VERSION = 0;
const int GEOMESH_HEADER_MESSAGE_TYPE = 2;
const int GEOMESH_HEADER_ROUTING_MODE = 3;
const int GEOMESH_HEADER_SOURCE_ADDRESS = 4;
const int GEOMESH_HEADER_SOURCE_LOCATION = 20;
const int GEOMESH_HEADER_DESTINATION_ADDRESS = 32;
const int GEOMESH_HEADER_DESTINATION_LOCATION = 48;
const int GEOMESH_HEADER_ROUTING_FACE_DISTANCE = 60;
const int GEOMESH_HEADER_ROUTING_FACE_RANGE = 64;
const int GEOMESH_PAYLOAD_START = 68;


// Position of fields in an IPv6 packet
// Relative to start of packet (usually GeoMesh payload start)
const int IPv6_SOURCE = 8;
const int IPv6_DESTINATION = 24;
const int IPv6_PAYLOAD_LENGTH = 4;
const int IPv6_PAYLOAD = 40;

// DHT routing table copy response field positions.
// Relative to payload start.
const int DHT_ROUTETABLE_COPY_NUMENTRIES = 0;
const int DHT_ROUTETABLE_COPY_ENTRIES_START = 4;
const int DHT_ROUTETABLE_COPY_ENTRYSIZE = 36;

// Positions of fields inside a DHT routetable copy response
// routing entry. Positions relative to entry start.
const int DHT_ROUTETABLE_ENTRY_ADDRESS = 0;
const int DHT_ROUTETABLE_ENTRY_LOCATION = 16;
const int DHT_ROUTETABLE_ENTRY_EXPIRES = 28;

// Field positions in a location lookup packet
// Relative to payload start.
const int LOCATION_LOOKUP_REQUESTER_ADDRESS = 0;
const int LOCATION_LOOKUP_REQUESTER_LOCATION = 16;
const int LOCATION_LOOKUP_TARGET_ADDRESS = 28;

// Field positions in a location lookup response
// Relative to payload start
const int LOCATION_LOOKUP_RESPONSE_TIME = 0;
const int LOCATION_LOOKUP_VALID_TIME = 0;

// Location info fields
// Relative to header start. (NOT PAYLOAD START!)
const int LOCATION_INFO_HOPS = 32;
const int LOCATION_INFO_CHECK = 34;

const int MAX_PAYLOAD_SIZE = MAX_PACKET_SIZE - GEOMESH_PAYLOAD_START;

class Packet; // Forward-declare for PacketPtr

typedef std::shared_ptr<Packet> PacketPtr;

/**
 * A class defining an arbitrary packet of information.
 *
 * Warning: avoid copying this packet too much, it is a relatively slow operation.
 */
class Packet {

    uint8_t data[MAX_PACKET_SIZE];
    int dataLength;
    // Pointer to the buffer position where the header starts. (usually just at the start of the buffer)
    uint8_t *header;
    // Pointer to a position within the buffer where the payload starts
    uint8_t *payload;

public:
    /**
     * Create a packet with a buffer of packet_size length, contents undefined.
     */
    Packet(size_t packet_size = MAX_PACKET_SIZE) : dataLength(packet_size),
                                                   header(data),
                                                   payload(header + GEOMESH_PAYLOAD_START) {

        assert(packet_size <= MAX_PACKET_SIZE);

        // Set protocol version to current
        *(reinterpret_cast<uint16_t *>(header + GEOMESH_HEADER_PROTOCOL_VERSION)) = htons(PROTOCOL_VERSION);
    }

    const uint8_t *getData() const {
        return data;
    }

    uint8_t *getHeader() const {
        return header;
    }

    uint8_t *getPayload() const {
        return payload;
    }

    int getPayloadLength() const {
        return dataLength - GEOMESH_PAYLOAD_START;
    }

    int getDataLength() const {
        return dataLength;
    }

    inline int getProtocolVersion() const {
        return *(reinterpret_cast<uint16_t *>(header + GEOMESH_HEADER_PROTOCOL_VERSION));
    }

    inline int getMessageType() const {
        return header[GEOMESH_HEADER_MESSAGE_TYPE];
    }

    inline float getPacketFaceRoutingClosestDistance() const {
        return *(reinterpret_cast<float *>(header + GEOMESH_HEADER_ROUTING_FACE_DISTANCE));
    }

    inline void setPacketFaceRoutingClosestDistance(double distance) {
        assert(CHAR_BIT * sizeof(float) == 32);

        *(reinterpret_cast<float *>(header + GEOMESH_HEADER_ROUTING_FACE_DISTANCE)) = distance;
    }

    inline float getPacketFaceRoutingSearchRange() const {
        return *(reinterpret_cast<float *>(header + GEOMESH_HEADER_ROUTING_FACE_RANGE));
    }

    inline void setPacketFaceRoutingSearchRange(double distance) {
        assert(CHAR_BIT * sizeof(float) == 32);

        *(reinterpret_cast<float *>(header + GEOMESH_HEADER_ROUTING_FACE_RANGE)) = distance;
    }

    inline void setMessageType(int msgType) {
        header[GEOMESH_HEADER_MESSAGE_TYPE] = msgType;
    }

    inline int getRoutingMode() const {
        // Need to apply bitmask since the face routing direction is in the leftmost bit
        return header[GEOMESH_HEADER_MESSAGE_TYPE];
    }

    inline uint16_t getLocationInfoHopCount() const {
        assert(getMessageType() == MSGTYPE_LOCATION_INFO);
        return ntohs(*(reinterpret_cast<uint16_t *>(header + LOCATION_INFO_HOPS)));
    }

    inline void setLocationInfoHopCount(const int &hops) {
        assert(getMessageType() == MSGTYPE_LOCATION_INFO);
        *(reinterpret_cast<uint16_t *>(header + LOCATION_INFO_HOPS)) = htons(hops);
    }

    inline void setRoutingMode(uint8_t routingMode) {
        header[GEOMESH_HEADER_MESSAGE_TYPE] = routingMode;
    }

    Location getSourceLocation() const {

        return Location::fromBytes(header + GEOMESH_HEADER_SOURCE_LOCATION);
    }

    void setSourceLocation(const Location &loc) {

        loc.toBytes(header + GEOMESH_HEADER_SOURCE_LOCATION);
    }

    /**
     * @return Whether the specified address is equal to the address in the IPv6 destination header field.
     */
    inline bool isDestination(const Address &address) const {
        assert(getMessageType() == MSGTYPE_IPv6);

        return memcmp(header + GEOMESH_HEADER_DESTINATION_ADDRESS, address.getBytes(), ADDRESS_LENGTH_OCTETS) == 0;
    }

    inline Address getDestinationAddress() const {

        Address addr;

        addr.setBytes(header + GEOMESH_HEADER_DESTINATION_ADDRESS);

        return addr;
    }

    inline Address getSourceAddress() const {

        Address addr;

        addr.setBytes(header + GEOMESH_HEADER_SOURCE_ADDRESS);

        return addr;
    }

    inline void setSource(const Address &address) const {
        memcpy(header + GEOMESH_HEADER_SOURCE_ADDRESS, address.getBytes(), ADDRESS_LENGTH_OCTETS);
    }

    inline void setDestination(const Address &address) const {
        memcpy(header + GEOMESH_HEADER_DESTINATION_ADDRESS, address.getBytes(), ADDRESS_LENGTH_OCTETS);
    }

    inline Location getDestinationLocation() {
        return Location::fromBytes(header + GEOMESH_HEADER_DESTINATION_LOCATION);
    }

    void setDestinationLocation(const Location &loc) {

        loc.toBytes(header + GEOMESH_HEADER_DESTINATION_LOCATION);
    }

    bool verifyLocationInformation();

    void recomputeLocationInfoChecksum();

    static PacketPtr createFromIPv6(const uint8_t *IPv6packet, size_t length,
                                    const Location &sourceLocation,
                                    const Location &destinationLocation);

    static PacketPtr createLocationInfoPacket(const Location &loc,
                                              const Address &addr);

    Packet(const uint8_t *data, size_t length);

    /**
     * Create a packet with a pre-filled header and an empty payload buffer of the specified size.
     */
    Packet(const Address &source,
           const Location &sourceLocation,
           const Address &destination,
           const Location &destinationLocation,
           int messageType,
           size_t payloadSize);
};


#endif //GEOMESH_PACKET_H
