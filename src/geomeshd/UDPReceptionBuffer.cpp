//
// Created by System Administrator on 2/29/16.
//

#include "UDPReceptionBuffer.hpp"
#include <cmath>

void UDPReceptionBuffer::removeOverlappingRanges() {

    // receivedRanges is sorted by starting index of the range.
    // For any two ranges (a,b) and (c,d), if they overlap,
    // merge them.
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
    // If this fragment belongs to a different packet than the one we were receiving,
    // reset the buffer.
    if (packetNumber != frag->getPacketNumber() || expectedSize != frag->getPacketLength()) {
        buffer.resize(frag->getPacketLength());
        expectedSize = frag->getPacketLength();
        packetNumber = frag->getPacketNumber();
        receivedRanges.clear();
    }

    // Invalidate, we need to re-check.
    valid = false;

    // Copy the data into the buffer
    memcpy(buffer.data() + frag->getFragmentStart(), frag->getPayloadData(), frag->getPayloadLength());

    // Mark the fragment's range as received.
    markReceived(frag->getFragmentStart(), frag->getFragmentStart() + frag->getPayloadLength());

    // Return whether a packet can be reconstructed at this moment
    return isFullPacketAvailable();
}

void UDPReceptionBuffer::markReceived(int begin, int end) {

    // Check whether the range was inserted or not.
    bool inserted = false;

    // To keep receivedRanges sorted, insert the new range before
    // the first range in the vector
    for (int index=0; index < receivedRanges.size(); ++index) {
        if (begin < receivedRanges[index].first) {
            receivedRanges.insert(receivedRanges.begin() + index,
                                  std::make_pair(begin,end));
            inserted = true;
            break;
        }
    }

    // If no range was found to insert it in front of, simply append it to the back
    if (! inserted) {
        receivedRanges.push_back(std::make_pair(begin,end));
    }

    // Remove the ranges that are overlapping, if any.
    removeOverlappingRanges();
}

/**
 * @return Whether the full packet has been received.
 */
bool UDPReceptionBuffer::isFullPacketAvailable() {

    // If a previous call has been made, return the cached result if not invalidated
    if (valid) return true;

    // If nothing has been received or the last range does not match the expected packet size,
    // the packet cannot be reconstructed
    if (receivedRanges.empty() || receivedRanges.back().second != expectedSize)
        return false;

    // If the first byte has not been received, quit
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
    return std::make_shared<Packet>(buffer.data(), buffer.size());
}
