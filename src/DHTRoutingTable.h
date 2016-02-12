//
// Created by System Administrator on 2/7/16.
//

#ifndef GEOMESH_DHTROUTINGTABLE_H
#define GEOMESH_DHTROUTINGTABLE_H

#include "constants.h"
#include "UniqueAddress.h"
#include "Packet.h"

struct DHTroutingTableEntry {
    Address address;
    Location location;
    time_t expires;

    DHTroutingTableEntry() : location(0,0), expires(0) {}
};

const int REDUNDANCY_LEVEL = 1;

const int NONREDUNDANT_ENTRIES = ADDRESS_LENGTH_OCTETS * 8;

class DHTRoutingTable {

public:
    DHTroutingTableEntry *getEntries() {
        return entries;
    }

private:
    DHTroutingTableEntry entries[NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL];

public:
    DHTRoutingTable() {
        for (int i=0; i< NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL; ++i) {
            entries[i].expires = 0; // 0 indicates invalid or null.
        }
    };

    void processRoutingTableCopy(PacketPtr message, int incomingIfaceID) {


    }

};


#endif //GEOMESH_DHTROUTINGTABLE_H
