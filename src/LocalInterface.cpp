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

#include "LocalInterface.hpp"

#include "AbstractLinkEndpoint.hpp"

#include "Packet.hpp"

#include "Router.hpp"

LocalInterface::LocalInterface(LocalPacketHandler& handler,
               LocationLookupManager& llm)
    : packetHandler(handler), llm(llm) {

  handler.addLocalPacketListener(
      std::bind(&LocalInterface::dataReceived,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3,
                std::placeholders::_4,
                std::placeholders::_5),
      MSGTYPE_IPv6);

      llm.addListener(std::bind(&LocalInterface::locationLookupCompleted,
          this,
          std::placeholders::_1,
          std::placeholders::_2,
          std::placeholders::_3));
}

void LocalInterface::locationLookupCompleted(Address addr, Location loc, time_t expires) {
    auto itr = waitingForLocationLookup.find(addr);

    if (itr != waitingForLocationLookup.end()) {
        for (std::vector<uint8_t>& dgram : itr->second) {
            packetHandler.sendFromLocal(MSGTYPE_IPv6, addr, loc, dgram.data(), dgram.size());
        }
    }
}

/**
 *
 */
bool LocalInterface::sendIPv6Message(const uint8_t *ipV6packet, int bytes) {

    Address destination = Address::fromBytes(ipV6packet + IPv6_DESTINATION);

    auto itr = llm.getContacts().findClosestEntry(destination);

    if (itr != llm.getContacts().end() && itr->address == destination) {

        packetHandler.sendFromLocal(MSGTYPE_IPv6, destination, itr->location, ipV6packet, bytes);

    } else {
        waitingForLocationLookup[destination].emplace_back(std::vector<uint8_t>(bytes));

        memcpy(waitingForLocationLookup[destination].back().data(), ipV6packet, bytes);

        llm.requestLocationLookup(destination);
    }

    return true;
}

void LocalInterface::dataReceived(int messageType,
                                  Address from,
                                  Location fromLocation,
                                  uint8_t* message,
                                  size_t messageSize) {

    assert(messageType == MSGTYPE_IPv6);

    dataReceivedHandler(message, messageSize);
}
