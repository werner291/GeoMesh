//
// Created by Werner Kroneman on 24-01-16.
//

#include "LocalInterface.h"

#include "AbstractInterface.h"

#include "constants.h"

#include "Router.h"

/**
 *
 */
bool LocalInterface::sendIPv6Message(const char* ipV6packet, int bytes) {

    DataBufferPtr msg(new std::vector<char>(MAX_PACKET_SIZE));

    setPacketData<int32_t>(PROTOCOL_VERSION_LOC, msg, PROTOCOL_VERSION);
    setPacketData<int32_t>(MESSAGE_TYPE, msg, MSGTYPE_PAYLOAD);

    Location(0,0).writeToPacket(DESTINATION_LOCATION, msg);

    setPacketData<int32_t>(TTL, msg, DEFAULT_TTL);

    std::memcpy(msg->data() + IPv6_START, ipV6packet, bytes);

    return router->handleMessage(msg, 0);
}

void LocalInterface::dataReceived(DataBufferPtr data) {

    // Decapitate the packet by shifting the original IPv6 data towards the front.
    // This will result in just an IPv6 packet, to be handles by the OS.
    memmove(data->data(),data->data(), data->size() - IPv6_START);

    dataReceivedHandler(data);
}