//
// Created by System Administrator on 2/29/16.
//

#ifndef GEOMESH_UDPFRAGMENT_H
#define GEOMESH_UDPFRAGMENT_H

#include <vector>
#include <netinet/in.h>
#include "../Packet.h"

class UDPFragment {

    std::vector<uint8_t> data;

    inline void setProtocolVersion(uint16_t version) {
        *reinterpret_cast<uint16_t*>(data.data()) = htons(version);
    }

public:

    /**
     * @param includesHeader Whether the provided data includes the header, or whether space should
     *                       be reserved in the buffer to fit a header later on.
     */
    UDPFragment(const uint8_t* buffer, const int bufferLength, bool includesHeader);

    inline int getProtocolVersion() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data()));
    }

    inline uint16_t getDestinationInterfaceID() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 2));
    }

    inline void setDestinationInterfaceID(uint16_t packetNum) {
        *reinterpret_cast<uint16_t*>(data.data() + 2) = htons(packetNum);
    }

    inline uint16_t getPacketNumber() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 4));
    }

    inline void setPacketNumber(uint16_t packetNum) {
        *reinterpret_cast<uint16_t*>(data.data() + 4) = htons(packetNum);
    }

    inline int getPacketLength() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 6));
    }

    inline void setPacketLength(int length) {
        assert(length <= MAX_PACKET_SIZE);
        *reinterpret_cast<uint16_t*>(data.data() + 6) = htons(length);
    }

    inline int getFragmentStart() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 8));
    }

    inline void setFragmentStart(uint16_t start) {
        *reinterpret_cast<uint16_t*>(data.data() + 8) = htons(start);
    }

    inline int getPayloadLength() {
        return ntohs(*reinterpret_cast<uint16_t*>(data.data() + 10));
    }

    inline void setFragmentLength(uint16_t length) {
        *reinterpret_cast<uint16_t*>(data.data() + 10) = htons(length);
    }

    uint8_t*getPayloadData() {
        return data.data() + 12;
    }

    uint8_t* getDataBuffer() {
        return data.data();
    }

    size_t getDataLength() {
        return data.size();
    }


};

typedef std::shared_ptr<UDPFragment> UDPFragmentPtr;

#endif //GEOMESH_UDPFRAGMENT_H
