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
    std::vector<uint8_t> route;
};

const int REDUNDANCY_LEVEL = 1;

const int NONREDUNDANT_ENTRIES = ADDRESS_LENGTH_OCTETS * 8;

class DHTRoutingTable {

    DHTroutingTableEntry entries[NONREDUNDANT_ENTRIES * REDUNDANCY_LEVEL];

    void processRoutingTableCopy(PacketPtr message, int incomingIfaceID) {


    }

};


#endif //GEOMESH_DHTROUTINGTABLE_H
