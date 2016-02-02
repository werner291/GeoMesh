//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_CONSTANTS_H
#define MESHNETSIM_CONSTANTS_H

#include <vector>

typedef std::shared_ptr<std::vector<char> > DataBufferPtr;

template<typename T> T getPacketData(int charposition, DataBufferPtr data) {
    return *(reinterpret_cast<T *>(data->data() + charposition));
}

template<typename T> void setPacketData(int charposition, DataBufferPtr data, T value) {
    *(reinterpret_cast<T *>(data->data() + charposition)) = value;
}

// TODO automate the variable allocation positions within the packet
// using the C++ preprocessor. This manual process is going to lead to mistakes.

const int PROTOCOL_VERSION = 0;
const int DEFAULT_TTL = 255; // IPv6 only has one byte for this, maybe GeoMesh should use its own?

typedef double location_scalar;
const int ADDRESS_LENGTH_OCTETS = 16; // 128-bit addresses, we're optimistic.

const int MSGTYPE_PAYLOAD = 0; // A regular message containing actual data to be transmitted
const int MSGTYPE_PEERINFO = 1; // A message containing information about the sender and their direct peers

const int ROUTING_GREEDY = 0;
const int ROUTING_XOR = 1;
const int ROUTING_FACE = 2;

const double FACE_ROUTE_RHR = 1;
const double FACE_ROUTE_LHR = 2;
const double FACE_ROUTING_RANGE_MULTIPLIER = 1.5;

// Common on all messages
const int PROTOCOL_VERSION_LOC  = 0;
const int MESSAGE_TYPE =          2;

// Payload messages
const int ROUTING_MODE             = 3;
const int DESTINATION_LOCATION = 4;
const int DESTINATION_LAT = 4; // Same as above since the location starts here.
const int DESTIANTION_LON = 8;

const int ROUTING_FACE_DISTANCE  = 16;
const int ROUTING_FACE_RANGE     = 20;

const int IPv6_START = 24;
const int DESTINATION_ADDRESS = IPv6_START + 24;
const int TTL = IPv6_START + 7;

// Peerinfo messages
const int PEERINFO_HOPS = MESSAGE_TYPE + 4;
const int PEERINFO_LOCATION_LON = PEERINFO_HOPS + 4;
const int PEERINFO_LOCATION_LAT = PEERINFO_LOCATION_LON + 8;
const int PEERINFO_ENTRY_UID = PEERINFO_LOCATION_LAT + 8;

const int MAX_PACKET_SIZE = 1500; // 1500 bytes

#endif //MESHNETSIM_CONSTANTS_H
