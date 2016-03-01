//
// Created by System Administrator on 2/29/16.
//

#include "UDPFragment.h"

#include <Exception>

#include "../Logger.h"

UDPFragment::UDPFragment(const uint8_t* buffer, int bufferLength, bool includesHeader) {

    if (includesHeader) {
        data.resize(bufferLength);

        memcpy(data.data(), buffer, bufferLength);

        if (getPayloadLength() + 12 > bufferLength) {
            throw std::runtime_error("Received fragment with invalid data length");
        }

        if (getFragmentStart() + getPayloadLength() > getPacketLength()) {
            throw std::runtime_error("Fragment does not fit in reported packet size.");
        }
    } else {
        data.resize(bufferLength + 12);

        memcpy(data.data() + 12, buffer, bufferLength);
    }

}