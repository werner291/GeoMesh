//
// Created by Werner Kroneman on 22-01-16.
//

#include "SimulatorLink.h"

int AbstractInterface::nextIfaceID = 1; // Not 0 since that's reserved. Need to use a proper interface manager.

bool SimulatorLink::sendData(std::shared_ptr<std::vector<char> > data) {

    // Create a copy, or we get the really wierd situation where opdating the number of
    // hops on one router will change all copies of the packet, even those in transit!
    sendQueue.push(std::make_shared<std::vector<char> >(data->begin(), data->end()));

    return true;
}

std::shared_ptr<std::vector<char> > SimulatorLink::pullNextInSendQueue() {

    std::shared_ptr<std::vector<char> > data = sendQueue.front();
    sendQueue.pop();
    return data;
}

bool SimulatorLink::hasNextInSendQueue() {
    return sendQueue.size() != 0;
}