//
// Created by System Administrator on 2/7/16.
//

#ifndef GEOMESH_DHTROUTINGTABLE_H
#define GEOMESH_DHTROUTINGTABLE_H

#include "UniqueAddress.h"
#include "Packet.h"
#include "my_htonll.h"

struct DHTroutingTableEntry {

    static const size_t SERIALIZED_SIZE = ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE + 8;

    Address address;
    Location location;
    time_t expires;

    DHTroutingTableEntry() : location(0,0), expires(0) {}

    static DHTroutingTableEntry fromBytes(const uint8_t* bytes) {
        DHTroutingTableEntry entry;

        entry.address.setBytes(bytes);
        entry.location = Location::fromBytes(bytes + ADDRESS_LENGTH_OCTETS);
        entry.expires = my_htonll(* reinterpret_cast<const uint64_t*>(bytes + ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE));

        return entry;
    }

    void toBytes(uint8_t* buffer) {
        memcpy(buffer, address.getBytes(), ADDRESS_LENGTH_OCTETS);
        location.toBytes((uint8_t*)buffer + ADDRESS_LENGTH_OCTETS);
        *(reinterpret_cast<uint64_t*>(buffer + ADDRESS_LENGTH_OCTETS + 8)) = my_htonll(expires);
    }
};

class DHTRoutingTable {

    static const int REDUNDANCY_LEVEL = 2;

    static const int NONREDUNDANT_ENTRIES = ADDRESS_LENGTH_OCTETS * 8;

    static const int NUM_ENTRIES = REDUNDANCY_LEVEL * NONREDUNDANT_ENTRIES;

    Address selfAddress;

public:
    DHTroutingTableEntry *getEntries() {
        return entries;
    }

private:
    DHTroutingTableEntry entries[NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL];

public:
    DHTRoutingTable(Address originAddress) : selfAddress(originAddress) {
        for (int i=0; i< NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL; ++i) {
            entries[i].expires = 0; // 0 indicates invalid or null.
        }
    };

    bool processEntrySuggestion(DHTroutingTableEntry entrySuggestion);

    void processRoutingTableCopy(const PacketPtr message, const int incomingIfaceID);

    std::vector<PacketPtr> getRoutingTableCopyMessages();

};


#endif //GEOMESH_DHTROUTINGTABLE_H
