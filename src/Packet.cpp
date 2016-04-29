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


#include "Packet.hpp"

const int FACE_ROUTE_DIRECTION_BITMASK = 128;


PacketPtr Packet::createFromIPv6(const uint8_t *IPv6packet, size_t length, const Location &sourceLocation,
                                 const Location &destinationLocation) {

    PacketPtr pack(new Packet(length + GEOMESH_PAYLOAD_START));

    pack->setMessageType(MSGTYPE_IPv6);

    // Check whether it will fit. (Should be checked before calling, this is just for debugging.)
    assert(length <= MAX_PAYLOAD_SIZE);

    // Copy the IPv6 packet into the payload of the GeoMesh packet
    memcpy(pack->payload, IPv6packet, length);

    // Copy the source and destination addresses into the GeoMesh packet header
    memcpy(pack->header + GEOMESH_HEADER_DESTINATION_ADDRESS, IPv6packet + IPv6_DESTINATION, ADDRESS_LENGTH_OCTETS);
    memcpy(pack->header + GEOMESH_HEADER_SOURCE_ADDRESS, IPv6packet + IPv6_SOURCE, ADDRESS_LENGTH_OCTETS);

    // Inscribe the source and destination location coordinates into the GeoMesh header
    sourceLocation.toBytes(pack->header + GEOMESH_HEADER_SOURCE_LOCATION);
    destinationLocation.toBytes(pack->header + GEOMESH_HEADER_DESTINATION_LOCATION);

    return pack;

}

PacketPtr Packet::createLocationInfoPacket(const Location &loc, const Address &addr) {
    PacketPtr pack(new Packet(500));

    pack->setMessageType(MSGTYPE_LOCATION_INFO);
    loc.toBytes(pack->header + GEOMESH_HEADER_SOURCE_LOCATION);

    memcpy(pack->header + GEOMESH_HEADER_SOURCE_ADDRESS, addr.getBytes(), ADDRESS_LENGTH_OCTETS);

    pack->setLocationInfoHopCount(1);
    pack->recomputeLocationInfoChecksum();

    return pack;
}

void Packet::recomputeLocationInfoChecksum() {// Compute checksum
    uint16_t *toVerify = reinterpret_cast<uint16_t *>(header);

    uint16_t sum = 0;

    for (int i = 0; i < LOCATION_INFO_CHECK / 2; i++) {
        sum += ntohs(toVerify[i]);
    }

    sum = 0 - sum; // These numbers are unsigned

    *(reinterpret_cast<uint16_t *>(header + LOCATION_INFO_CHECK)) = htons(sum);
}

bool Packet::verifyLocationInformation() {

    assert(getMessageType() == MSGTYPE_LOCATION_INFO);

    uint16_t *toVerify = reinterpret_cast<uint16_t *>(header);

    uint16_t sum = 0;

    for (int i = 0; i < LOCATION_INFO_CHECK / 2 + 1; i++) {
        sum += ntohs(toVerify[i]);
    }

    return sum == 0;

}


Packet::Packet(const uint8_t *data, size_t length) : Packet(length) {
    if (data != nullptr) {
        memcpy(Packet::data, data, length);
    }
}

Packet::Packet(const Address &source,
                 const Location &sourceLocation,
                 const Address &destination,
                 const Location &destinationLocation,
                 int messageType,
                 size_t payloadSize) : Packet(GEOMESH_PAYLOAD_START + payloadSize) {

    setMessageType(messageType);

    setDestination(destination);
    setDestinationLocation(destinationLocation);

    setSource(source);
    setSourceLocation(sourceLocation);
}
