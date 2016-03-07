//
// Created by System Administrator on 2/29/16.
//

#include "UDPReceptionBuffer.h"
#include <cmath>

void UDPReceptionBuffer::removeOverlappingRanges() {

    for (int index=0; index+1 < receivedRanges.size(); ++index) {
        // We have overlap
        if (receivedRanges[index].second >= receivedRanges[index+1].first) {
            receivedRanges[index].second = std::max(receivedRanges[index].second,
                                                    receivedRanges[index+1].second);
            receivedRanges.erase(receivedRanges.begin() + index + 1);
            --index; // Stay at the same position.
        }
    }
}

bool UDPReceptionBuffer::receive(UDPFragmentPtr frag) {

    if (packetNumber != frag->getPacketNumber() || expectedSize != frag->getPacketLength()) {
        buffer.resize(frag->getPacketLength());
        expectedSize = frag->getPacketLength();
        packetNumber = frag->getPacketNumber();
        receivedRanges.clear();
    }

    valid = false;

    memcpy(buffer.data() + frag->getFragmentStart(), frag->getPayloadData(), frag->getPayloadLength());

    markReceived(frag->getFragmentStart(), frag->getFragmentStart() + frag->getPayloadLength());

    return isFullPacketAvailable();
}

void UDPReceptionBuffer::markReceived(int begin, int end) {

    bool inserted = false;

    for (int index=0; index < receivedRanges.size(); ++index) {
        if (begin < receivedRanges[index].first) {
            receivedRanges.insert(receivedRanges.begin() + index,
                                  std::make_pair(begin,end));
            inserted = true;
            break;
        }
    }

    if (! inserted) {
        receivedRanges.push_back(std::make_pair(begin,end));
    }

    removeOverlappingRanges();
}

/**
 * @return Whether the full packet has been received.
 */
bool UDPReceptionBuffer::isFullPacketAvailable() {

    if (valid) return true;

    if (receivedRanges.empty() || receivedRanges.back().second != expectedSize)
        return false;

    if (receivedRanges.front().first != 0)
        return false;

    int lastEnd = 0;

    for (auto itr = receivedRanges.begin(); itr != receivedRanges.end(); ++itr) {
        if (itr->first > lastEnd)
            return false; // There's a gap.

        lastEnd = itr->second;
    }

    valid = true;

    return true;

}

PacketPtr UDPReceptionBuffer::getReconstructedPacket() {
    assert(isFullPacketAvailable());
    return Packet::createFromData(buffer.data(), buffer.size());
}