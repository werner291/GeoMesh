//
// Created by System Administrator on 2/29/16.
//

#ifndef GEOMESH_UDPFRAGMENT_H
#define GEOMESH_UDPFRAGMENT_H


class UDPFragment {

    std::vector<uint8_t> data;

public:

    UDPFragment(uint8_t* buffer, int bufferLength);

    inline int getProtocolVersion() {
        return ntohs(reinterpret_cast<uint16_t>(data.data()));
    }

    inline int getPacketLength() {
        return ntohs(reinterpret_cast<uint16_t>(data.data() + 2));
    }

    inline uint16_t getPacketNumber() {
        return ntohs(reinterpret_cast<uint16_t>(data.data() + 4));
    }

    inline uint16_t getDestinationInterfaceID() {
        return ntohs(reinterpret_cast<uint16_t>(data.data() + 6));
    }

    inline int getFragmentStart() {
        return ntohs(reinterpret_cast<uint16_t>(data.data() + 8));
    }

    inline int getFragmentLength() {
        return ntohs(reinterpret_cast<uint16_t>(data.data() + 10));
    }

    uint8_t* getData() {
        return data.data();
    }

    size_t getDataLength() {
        return data.size();
    }
};

typedef std::shared_ptr<UDPFragment> UDPFragmentPtr;

#endif //GEOMESH_UDPFRAGMENT_H
