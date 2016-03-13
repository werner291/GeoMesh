//
// Created by Werner Kroneman on 22-01-16.
//

#include "SimulatorLink.h"

bool SimulatorLink::sendData(PacketPtr data) {

    // Create a copy, or we get the really wierd situation where updating the number of
    // hops on one router will change all copies of the packet, even those in transit!

    PacketPtr ptr = Packet::createFromData(data->getData(), data->getDataLength());

    sendQueue.push(ptr);

    return true;
}

PacketPtr SimulatorLink::pullNextInSendQueue() {

    PacketPtr data = sendQueue.front();
    sendQueue.pop();
    return data;
}

bool SimulatorLink::hasNextInSendQueue() {
    return sendQueue.size() != 0;
}
