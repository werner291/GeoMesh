//
// Created by Werner Kroneman on 22-01-16.
//

#include "SimulatorInterface.h"

int AbstractInterface::nextIfaceID = 1; // Not 0 since that's reserved. Need to use a proper interface manager.

bool SimulatorInterface::sendData(std::shared_ptr<std::vector<char> > data) {
    sendQueue.push(data);

    return true;
}

std::shared_ptr<std::vector<char> > SimulatorInterface::pullNextInSendQueue() {

    std::shared_ptr<std::vector<char> > data = sendQueue.front();
    sendQueue.pop();
    return data;
}

bool SimulatorInterface::hasNextInSendQueue() {
    return sendQueue.size() != 0;
}
