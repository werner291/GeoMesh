//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_CONSTANTS_H
#define MESHNETSIM_CONSTANTS_H

// TODO automate the variable allocation positions within the packet
// using the C++ preprocessor. This manual process is going to lead to mistakes.

const int PROTOCOL_VERSION = 0;
const int DEFAULT_TTL = 100;

typedef double location_scalar;
const int ADDRESS_LENGTH_OCTETS = 16; // 128-bit addresses, we're optimistic.

// Common on all messages
const int PROTOCOL_VERSION_LOC  = 0;
const int MESSAGE_TYPE = PROTOCOL_VERSION_LOC + 4;

const int MSGTYPE_PAYLOAD = 0; // A regular message containing actual data to be transmitted
const int MSGTYPE_PEERINFO = 1; // A message containing information about the sender and their direct peers

const int ROUTING_GREEDY = 0;
const int ROUTING_XOR = 1;
const int ROUTING_FACE = 2;

const double FACE_ROUTE_RHL = 1;
const double FACE_ROUTE_LHR = 2;
const double FACE_ROUTING_RANGE_MULTIPLIER = 1.5;

// Payload messages
const int ROUTING_MODE           = MESSAGE_TYPE  + 4;
const int ROUTING_FACE_START_X   = ROUTING_MODE  + 4;
const int ROUTING_FACE_START_Y   = ROUTING_FACE_START_X  + 8;
const int ROUTING_FACE_RANGE     = ROUTING_FACE_START_Y  + 8;
const int ROUTING_FACE_DIRECTION = ROUTING_FACE_RANGE + 8;
const int LOCATION_COORDINATE_X  = ROUTING_FACE_DIRECTION  + 4;
const int LOCATION_COORDINATE_Y  = LOCATION_COORDINATE_X + 8;
const int DESTINATION_ID =        LOCATION_COORDINATE_Y + 8;
const int TTL =                   DESTINATION_ID + 8;
const int PAYLOAD_START =         TTL + 4;

// Peerinfo messages
const int PEERINFO_HOPS = MESSAGE_TYPE + 4;
const int PEERINFO_ENTRY_LOCATION_X = PEERINFO_HOPS + 4;
const int PEERINFO_ENTRY_LOCATION_Y = PEERINFO_ENTRY_LOCATION_X + 8;
const int PEERINFO_ENTRY_UID = PEERINFO_ENTRY_LOCATION_Y + 8;

#endif //MESHNETSIM_CONSTANTS_H
