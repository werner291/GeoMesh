//
// Created by System Administrator on 1/30/16.
//

#include <assert.h>
#include "PacketFunctions.h"
#include "constants.h"

const int FACE_ROUTE_DIRECTION_BITMASK = 128;

int getPacketFaceDirection(DataBufferPtr packet) {

    if ((packet->data()[3] & FACE_ROUTE_DIRECTION_BITMASK) == 0) { // Isolate the leftmost bit
        return FACE_ROUTE_RHR;
    } else {
        return FACE_ROUTE_LHR;
    }

}

void setPacketFaceRoutingDirection(DataBufferPtr packet, int direction) {
    if (direction == FACE_ROUTE_RHR) { // Isolate the leftmost bit
        packet->data()[3] |= FACE_ROUTE_DIRECTION_BITMASK;  // Set leftmost bit to 1
    } else {
        packet->data()[3] &= ~FACE_ROUTE_DIRECTION_BITMASK; // Set leftmost bit to 0
    }
}

void setPacketFaceRoutingClosestDistance(DataBufferPtr data, double distance) {
    assert(CHAR_BIT * sizeof (float) == 32);

    setPacketData<float>(ROUTING_FACE_DISTANCE, data, distance);
}

void setPacketFaceRoutingSearchRange(DataBufferPtr data, double distance) {
    assert(CHAR_BIT * sizeof (float) == 32);

    setPacketData<float>(ROUTING_FACE_RANGE, data, distance);
}

float getPacketFaceRoutingClosestDistance(DataBufferPtr data, double distance) {
    return getPacketData<float>(ROUTING_FACE_DISTANCE, data);
}

float getPacketFaceRoutingSearchRange(DataBufferPtr data, double distance) {
    return getPacketData<float>(ROUTING_FACE_RANGE, data);
}
