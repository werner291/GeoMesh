//
// Created by System Administrator on 2/7/16.
//

#ifndef GEOMESH_DHTROUTINGTABLE_H
#define GEOMESH_DHTROUTINGTABLE_H

#include "constants.h"
#include "UniqueAddress.h"
#include "Packet.h"

struct DHTroutingTableEntry {

    static const size_t SERIALIZED_SIZE = ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE + 8;

    Address address;
    Location location;
    time_t expires;

    DHTroutingTableEntry() : location(0,0), expires(0) {}

    static DHTroutingTableEntry fromBytes(const void* bytes) {
        DHTroutingTableEntry entry;

        memcpy(entry.address.getBytes(),bytes,ADDRESS_LENGTH_OCTETS);
        entry.location = Location::fromBytes(bytes + ADDRESS_LENGTH_OCTETS);
        entry.expires = ntohll(* reinterpret_cast<uint64_t*>(bytes + ADDRESS_LENGTH_OCTETS + Location::SERIALIZED_SIZE));

        return entry;
    }

    void toBytes(void* buffer) {
        memcpy(buffer, address.getBytes(), ADDRESS_LENGTH_OCTETS);
        location.toBytes(buffer + ADDRESS_LENGTH_OCTETS);
        *(reinterpret_cast<uint64_t*>(bytes + ADDRESS_LENGTH_OCTETS + 8)) = htonll(expires);
    }
};

class DHTRoutingTable {

    const int REDUNDANCY_LEVEL = 2;

    const int NONREDUNDANT_ENTRIES = ADDRESS_LENGTH_OCTETS * 8;

    const int NUM_ENTRIES = REDUNDANCY_LEVEL * NONREDUNDANT_ENTRIES;

    Address selfAddress;

public:
    DHTroutingTableEntry *getEntries() {
        return entries;
    }

private:
    DHTroutingTableEntry entries[NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL];

public:
    DHTRoutingTable(Address originAddress) : originAddress(selfAddress) {
        for (int i=0; i< NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL; ++i) {
            entries[i].expires = 0; // 0 indicates invalid or null.
        }
    };

    bool processEntrySuggestion(DHTroutingTableEntry entrySuggestion);

    void processRoutingTableCopy(const PacketPtr message, const int incomingIfaceID);

    PacketPtr getRoutingTableCopyMessage();

};


#endif //GEOMESH_DHTROUTINGTABLE_H
