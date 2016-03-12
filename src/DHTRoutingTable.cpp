//
// Created by System Administrator on 2/7/16.
//

#include "DHTRoutingTable.h"

bool DHTRoutingTable::processEntrySuggestion(DHTroutingTableEntry entrySuggestion) {
    
    AddressDistance dist = entrySuggestion.address.xorDistanceTo(selfAddress);
    
    int slotNum = dist.getDHTSlotNumber();
    
    // Insert and keep the sorted order by going from front to back and swapping those that need to move.
    for (int insertPos = slotNum * REDUNDANCY_LEVEL; insertPos < (slotNum+1) * REDUNDANCY_LEVEL; ++insertPos) {

        const AddressDistance &entryDist = entries[insertPos].address.xorDistanceTo(selfAddress);

        if (dist < entryDist) {
            std::swap(entries[insertPos],entrySuggestion)
        }
    }
    
}

void DHTRoutingTable::processRoutingTableCopy(PacketPtr message, int incomingIfaceID) {

    int numEntries = ntohl(*reinterpret_cast<uint32_t*>message->getPayload());

    if (numEntries * DHTroutingTableEntry::SERIALIZED_SIZE > message->getPayloadLength() - 4) {
        throw runtime_exception("Number of entries too large to fit payload.");
    }

    for (int entry = 0; entry < numEntries; ++entry) {

        void* entryStart = message->getPayload() + 4 + entry * DHTroutingTableEntry::SERIALIZED_SIZE;

        DHTroutingTableEntry entry = DHTroutingTableEntry::fromBytes(entryStart);

        processEntrySuggestion(entry);

    }

}

std::vector<PacketPtr> DHTRoutingTable::getRoutingTableCopyMessages() {

    std::vector<PacketPtr> messages;

    messages.push_back(new Packet(MSGTYPE_DHT_ROUTETABLE_COPY));

    int entriesInBatch = 0;

    for (int index = 0; index < NUM_ENTRIES; index += 1) {

        entriesInBatch += 1;

        if (entriesInBatch * DHTroutingTableEntry::SERIALIZED_SIZE + 4 > MAX_PAYLOAD_SIZE) {
            messages.push_back(new Packet(MSGTYPE_DHT_ROUTETABLE_COPY));
            entriesInBatch = 1;
        }

        DHTroutingTableEntry& entry = entries[index];

        (*reinterpret_cast<uint32_t*>(packet->getPayload())) = entriesInBatch;

        void* entryStart = messages.back()->getPayload() + 4 + entriesInBatch * DHTroutingTableEntry::SERIALIZED_SIZE;

        entry.toBytes(entryStart);

    }

    return messages;

}