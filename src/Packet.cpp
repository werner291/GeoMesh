//
// Created by System Administrator on 2/5/16.
//

#include "Packet.h"

const int FACE_ROUTE_DIRECTION_BITMASK = 128;


PacketPtr Packet::createFromIPv6(const uint8_t *IPv6packet, size_t length, const Location &sourceLocation,
                                 const Location &destinationLocation) {

    PacketPtr pack(new Packet(MSGTYPE_IPv6));

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
    PacketPtr pack(new Packet(MSGTYPE_LOCATION_INFO));

    loc.toBytes(pack->header + GEOMESH_HEADER_SOURCE_LOCATION);

    memcpy(pack->header + GEOMESH_HEADER_SOURCE_ADDRESS, addr.getBytes(), ADDRESS_LENGTH_OCTETS);

    pack->setLocationInfoHopCount(1);

    // Compute checksum
    uint16_t *toVerify = reinterpret_cast<uint16_t *>(pack->header);

    uint16_t sum = 0;

    for (int i = 0; i < LOCATION_INFO_CHECK / 2; i++) {
        sum += ntohs(toVerify[i]);
    }

    sum = 0 - sum; // These numbers are unsigned

    *(reinterpret_cast<uint16_t *>(pack->header + LOCATION_INFO_CHECK)) = htons(sum);

    return pack;
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

// Perhaps just a constructor would be better here?
PacketPtr Packet::createFromData(const uint8_t *data, size_t length) {

    assert(length <= MAX_PACKET_SIZE);

    PacketPtr pack(new Packet(0));

    memcpy(pack->data, data, length);

    return pack;

}