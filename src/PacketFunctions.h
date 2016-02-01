//
// Created by System Administrator on 1/30/16.
//

#include "constants.h"

int getPacketFaceDirection(DataBufferPtr packet);

void setPacketFaceRoutingDirection(DataBufferPtr packet, int direction);

void setPacketFaceRoutingClosestDistance(DataBufferPtr data, double distance);

float getPacketFaceRoutingClosestDistance(DataBufferPtr data);

void setPacketFaceRoutingSearchRange(DataBufferPtr data, double distance);

float getPacketFaceRoutingSearchRange(DataBufferPtr data);