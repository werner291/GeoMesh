//
// Created by System Administrator on 2/7/16.
//

#include "LocationLookupManager.hpp"
#include <exception>
#include "Router.hpp"
#include <string.h>
#include "VirtualLocationManager.hpp"

bool LocationLookupManager::processEntrySuggestion(const Address& address, const Location& loc, time_t expires) {

    addEntry(address, loc, expires);

    waitingForLookup.erase(address);

    for (Listener listener : updateListeners) {
        listener(address, loc, expires);
    }

    return true;
}

void LocationLookupManager::addEntry(const Address& address, const Location& loc, time_t expires) {

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

    waitingForLookup.insert(toLookUp);

    uint8_t message[FIND_CLOSEST_MESSAGE_SIZE];

    LocationLookupManager::writeLookupMessage(message, selfAddress, locationMgr.getLocation(), toLookUp);

    handleDHTPacket(MSGTYPE_DHT_FIND_CLOSEST,
                    selfAddress,
                    locationMgr.getLocation(),
                    message,
                    FIND_CLOSEST_MESSAGE_SIZE);

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
                                             const VirtualLocationManager &locationMgr)
        : localHandler(localHandler),
          selfAddress(selfAddress),
          locationMgr(locationMgr) {

    for (int msgType : {MSGTYPE_DHT_FIND_CLOSEST,
                        MSGTYPE_DHT_FIND_RESPONSE,
                        MSGTYPE_DHT_LEAVE}) {
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
                                            Location fromLocation,
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
                Address requesterAddress = Address::fromBytes(message + FIND_CLOSEST_REQUESTER);
                Location requesterLocation = Location::fromBytes(message + FIND_CLOSEST_REQUESTER_LOCATION);

                localHandler.sendFromLocal(MSGTYPE_DHT_FIND_RESPONSE,
                                           requesterAddress,
                                           requesterLocation,
                                           message,
                                           messageSize);
            }

        }
            break;
        default:
            Logger::log(LogLevel::WARN, "LocationLookupManager.handleDHTPacket received message with unknown type "
                                        + std::to_string(messageType));
            break;
    }

}
