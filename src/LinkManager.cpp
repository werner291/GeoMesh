/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

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
