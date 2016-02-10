//
// Created by Werner Kroneman on 24-01-16.
//

#include "LocalInterface.h"

#include "AbstractInterface.h"

#include "Packet.h"

#include "Router.h"

/**
 *
 */
bool LocalInterface::sendIPv6Message(const uint8_t *ipV6packet, int bytes) {

    PacketPtr packet = Packet::createFromIPv6(ipV6packet, bytes, router->getVirtualLocation(), Location(0, 0));

    return router->handleMessage(packet, 0);
}

void LocalInterface::dataReceived(PacketPtr data) {

    dataReceivedHandler(data);
}