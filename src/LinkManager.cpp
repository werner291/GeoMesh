//
// Created by Werner Kroneman on 23-01-16.
//

#include "LinkManager.h"
#include "Logger.h"
#include "Router.h"

bool LinkManager::sendPacket(const DataBufferPtr &data, int iFace) {
    auto itr = mInterfaces.find(iFace);

    if (itr != mInterfaces.end()) {
        return itr->second->sendData(data);
    } else {
        Logger::log(LogLevel::WARN, "Trying to send data to non-existent interface " + std::to_string(iFace));
        return false;
    }
}

void LinkManager::connectInterface(std::shared_ptr<AbstractInterface> iFace) {

    mInterfaces.insert(std::make_pair(iFace->getInterfaceId(), iFace));
    iFace->setDataReceivedCallback(std::bind(&Router::handleMessage, // Ugly...
                                             router, std::placeholders::_1, std::placeholders::_2));

    for (auto listener : linkEventListeners) {
        listener(*iFace, LINK_CREATED);
    }
}

void LinkManager::broadcastMessage(const DataBufferPtr &data) {
    for (auto pair : mInterfaces) {
        pair.second->sendData(data);
    }
}