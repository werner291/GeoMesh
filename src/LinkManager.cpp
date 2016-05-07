//
// Created by Werner Kroneman on 23-01-16.
//

#include "LinkManager.hpp"
#include "Logger.hpp"
#include "Router.hpp"

bool LinkManager::sendPacket(const PacketPtr &data, int iFace) {
    auto itr = mInterfaces.find(iFace);

    if (itr != mInterfaces.end()) {
        return itr->second->sendData(data);
    } else {
        Logger::log(LogLevel::WARN, "Trying to send data to non-existent interface " + std::to_string(iFace));
        return false;
    }
}

void LinkManager::connectInterface(std::shared_ptr<AbstractLinkEndpoint> iFace) {

    mInterfaces.insert(std::make_pair(iFace->getInterfaceId(), iFace));
    iFace->setDataReceivedCallback(routeInboundPacket);

    for (auto listener : linkEventListeners) {
        listener(iFace, LINKEVENT_CREATED);
    }
}

void LinkManager::broadcastMessage(const PacketPtr &data) {
    for (auto pair : mInterfaces) {
        pair.second->sendData(data);
    }
}
