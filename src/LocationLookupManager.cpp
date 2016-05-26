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

#include "LocationLookupManager.hpp"
#include <exception>
#include "Router.hpp"
#include <string.h>
#include "VirtualLocationManager.hpp"

bool LocationLookupManager::processEntrySuggestion(const Address& address, const GPSLocation& loc, time_t expires) {

	if (address != selfAddress) {
		// Do not add ourselves as a contact
        addEntry(address, loc, expires);
        Logger::log(LogLevel::DEBUG,
               "Added contact " + address.toString() 
                + " at " + loc.getDescription());
	}

    waitingForLookup.erase(address);

    for (Listener listener : updateListeners) {
        listener(address, loc, expires);
    }

    return true;
}

void LocationLookupManager::addEntry(const Address& address, const GPSLocation& loc, time_t expires) {

    contacts.insert(ContactsSet::Entry(address,loc,expires));
}

void LocationLookupManager::refreshRoutingTable() {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

    memcpy(bytes,selfAddress.getBytes(),ADDRESS_LENGTH_OCTETS);

    for (int byte=0; byte < ADDRESS_LENGTH_OCTETS; ++byte) {

        for (int bit = 0; bit <= 7; ++bit) {

            uint8_t bitmask = 1 << bit;

            bytes[byte] ^= bitmask;

            Address desiredPeer = Address::fromBytes(bytes);

            bytes[byte] ^= bitmask; // Flip it back.

            requestLocationLookup(desiredPeer);

        }

    }

}

void LocationLookupManager::requestLocationLookup(const Address& toLookUp) {

	Logger::log(LogLevel::DEBUG, "Requested location lookup for address " 
            + toLookUp.toString());

    waitingForLookup.insert(toLookUp);

    if (! contacts.empty() ) 
    {
        uint8_t message[FIND_CLOSEST_MESSAGE_SIZE];

        writeLookupMessage(message, selfAddress, locationMgr.getLocation(),
                toLookUp);

        auto itr = contacts.findClosestEntry(toLookUp);

        localHandler.sendFromLocal(MSGTYPE_DHT_FIND_CLOSEST,
                                   itr->address,
                                   itr->location,
                                   message,
                                   FIND_CLOSEST_MESSAGE_SIZE);

    } 
    else 
    {
	    Logger::log(LogLevel::INFO, "Location Lookup Manager has no contacts,"
               " will perform request when contacts acquired.");
    }
    
}

/*
void LocationLookupManager::keepAlive() {

    PacketPtr packet = std::make_shared<Packet>(router.getAddress(),
                                        router.getVirtualLocation(),
                                        closestEntry.address,
                                        closestEntry.location,
                                        MSGTYPE_DHT_KEEP_ALIVE,
                                        0);

}*/

LocationLookupManager::LocationLookupManager(LocalPacketHandler& localHandler,
                                             const Address &selfAddress,
                                             const VirtualLocationManager &locationMgr,
                                             ContactsSet& contacts)
        : selfAddress(selfAddress), locationMgr(locationMgr),
          localHandler(localHandler), contacts(contacts)
{
    for (int msgType : {MSGTYPE_DHT_FIND_CLOSEST,
                        MSGTYPE_DHT_FIND_RESPONSE,
                        MSGTYPE_DHT_LEAVE,
                        MSGTYPE_LOCATION_INFO}) {
        localHandler.addLocalPacketListener(std::bind(&LocationLookupManager::handleDHTPacket,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2,
                                                      std::placeholders::_3,
                                                      std::placeholders::_4,
                                                      std::placeholders::_5),
                                            msgType);
    }
}

void LocationLookupManager::handleDHTPacket(int messageType,
                                            Address from,
                                            GPSLocation fromLocation,
                                            uint8_t* message,
                                            size_t messageSize) {

    switch (messageType) {
        case MSGTYPE_DHT_FIND_CLOSEST: {

            Address query = Address::fromBytes(message + FIND_CLOSEST_QUERY);

            auto itr = contacts.findClosestEntry(query);

            if (itr != contacts.end() && selfAddress.xorDistanceTo(query) > itr->address.xorDistanceTo(query)) {
                // The contact is closer in xor space to the query, forward the query to the contact
                // TODO: route choice number
                localHandler.sendFromLocal(messageType,
                                           itr->address,
                                           itr->location,
                                           message,
                                           messageSize);
            } else {
                // No contacts are closer than the current node, respond with our own contact info.
                Address requesterAddress = Address::fromBytes(message 
                                               + FIND_CLOSEST_REQUESTER);
                GPSLocation requesterLocation = GPSLocation::fromBytes(message
                                      + FIND_CLOSEST_REQUESTER_LOCATION);

                localHandler.sendFromLocal(MSGTYPE_DHT_FIND_RESPONSE,
                                           requesterAddress,
                                           requesterLocation,
                                           message,
                                           messageSize);
                processEntrySuggestion(requesterAddress,requesterLocation,0);
            }

        }
            break;
        case MSGTYPE_DHT_FIND_RESPONSE: {
            auto itr = waitingForLookup.find(from);

            if (itr == waitingForLookup.end()) {
            // Perhaps the node was not reachable?
            // Put it in the contacts anyway
            // TODO: Add a request ID to the message and
            // check whether the message was asked for or not.
            } else {
                Logger::log(LogLevel::DEBUG, 
                        std::string("Location lookup completed. ")
                        + from.toString() + " is at " 
                        + fromLocation.getDescription());

                for (Listener& listener : updateListeners) {
                    listener(from,fromLocation,0); // TODO better expiry time
                }

                waitingForLookup.erase(itr);
            }
            processEntrySuggestion(from,fromLocation,0);
            }
            break;
        case MSGTYPE_LOCATION_INFO: {

            processEntrySuggestion(from,fromLocation,0);
        } break;
        default:
            Logger::log(LogLevel::WARN,
                        "LocationLookupManager.handleDHTPacket received "
                        "message with unknown type "
                                        + std::to_string(messageType));
            break;
    }

}
