//
// Created by System Administrator on 2/29/16.
//

#ifndef GEOMESH_UDPRECEPTIONBUFFER_H
#define GEOMESH_UDPRECEPTIONBUFFER_H

#include "UDPFragment.h"

#include "../Packet.h"

class UDPReceptionBuffer {

    int packetNumber;
    int expectedSize;

    bool valid;

    std::vector< std::pair<int,int> > receivedRanges;

    std::vector<uint8_t> buffer;

    void removeOverlappingRanges();

public:

    UDPReceptionBuffer() : valid(false), expectedSize(0), packetNumber(0) {}

    /**
     * Receive and store a fragment of data.
     */
    bool receive(UDPFragmentPtr frag);

    /**
     * @return Whether the full packet has been received.
     */
    bool isFullPacketAvailable();

    PacketPtr getReconstructedPacket();

    void markReceived(int begin, int end);
};


#endif //GEOMESH_UDPRECEPTIONBUFFER_H
