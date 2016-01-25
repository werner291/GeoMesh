//
// Created by Werner Kroneman on 24-01-16.
//

#include "LocalInterface.h"

#include "AbstractInterface.h"

#include "constants.h"

#include "Router.h"

bool LocalInterface::sendMessage(const std::vector<char> &address, DataBufferPtr body, Location recipientLocation) {

    if (body->size() > 512 - PAYLOAD_START) {
        return false;
    }

    DataBufferPtr msg(new std::vector<char>(512));

    setPacketData<int32_t>(PROTOCOL_VERSION_LOC, msg, PROTOCOL_VERSION);


    setPacketData<int32_t>(MESSAGE_TYPE, msg, MSGTYPE_PAYLOAD);

    setPacketData<double>(LOCATION_COORDINATE_X, msg, recipientLocation.X);
    setPacketData<double>(LOCATION_COORDINATE_Y, msg, recipientLocation.Y);

    //setPacketData<double>(DESTINATION_ID, data, endNodeID)
    setPacketData<int32_t>(TTL, msg, DEFAULT_TTL);

    //std::memcpy(msg->data() + PAYLOAD_START, body->data(), body->size());

    return router->handleMessage(msg, 0);
}