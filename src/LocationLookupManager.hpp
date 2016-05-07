//
// Created by System Administrator on 2/7/16.
//

#ifndef GEOMESH_DHTROUTINGTABLE_H
#define GEOMESH_DHTROUTINGTABLE_H

#include "UniqueAddress.hpp"
#include "Packet.hpp"
#include "my_htonll.hpp"
#include "VirtualLocationManager.hpp"
#include "LocalPacketHandler.hpp"
#include "ContactsSet.hpp"
#include <exception>
#include <set>

static const int FIND_CLOSEST_QUERY = ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE;

static const int FIND_CLOSEST_REQUESTER = 0;

static const int FIND_CLOSEST_REQUESTER_LOCATION = ADDRESS_LENGTH_OCTETS;

static const int FIND_CLOSEST_MESSAGE_SIZE = ADDRESS_LENGTH_OCTETS
                                                + Location::SERIALIZED_SIZE
                                                + ADDRESS_LENGTH_OCTETS;

class LocalPacketHandler;

class LocationLookupManager {

    static const int REDUNDANCY_LEVEL = 2;

    typedef std::function< void (const Address& address, const Location& loc,
            time_t expires) > Listener;

    std::vector< Listener > updateListeners;

    Address selfAddress;

    ContactsSet contacts;

    std::set<Address> waitingForLookup;

    const VirtualLocationManager& locationMgr;

    LocalPacketHandler& localHandler;

    void handleDHTPacket(int messageType,
                         Address from,
                         Location fromLocation,
                         uint8_t* message,
                         size_t messageSize);

public:
    LocationLookupManager(LocalPacketHandler& localHandler,
                          const Address &selfAddress,
                          const VirtualLocationManager &locationMgr);

    bool processEntrySuggestion(const Address& address, const Location& loc, time_t expires);

    void processRoutingTableCopy(const uint8_t* bytes, size_t length);

    void requestLocationLookup(const Address& toLookUp);

    void addListener(const Listener& listener) {
        updateListeners.push_back(listener);
    }

    void removeAllListeners() {
	    updateListeners.clear();
    }

    void refreshRoutingTable();

    void prune(){};

    void keepAlive();

    const ContactsSet &getContacts() const {
        return contacts;
    }

    /**
     * Add this entry directly to the routing table.
     */
    void addEntry(const Address& address, const Location& loc, time_t expires);

    static void writeLookupMessage(uint8_t* buffer, Address requester, Location requesterLocation, Address query) {
        memcpy(buffer, requester.getBytes(), ADDRESS_LENGTH_OCTETS);

        requesterLocation.toBytes(buffer + ADDRESS_LENGTH_OCTETS);

        memcpy(buffer + ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE,
               query.getBytes(),
               ADDRESS_LENGTH_OCTETS);
    }
};



#endif //GEOMESH_DHTROUTINGTABLE_H
