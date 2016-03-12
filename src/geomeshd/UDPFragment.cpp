//
// Created by System Administrator on 2/29/16.
//

#include "UDPFragment.h"

#include <exception>

#include "../Logger.h"

UDPFragment::UDPFragment(const uint8_t* buffer, int bufferLength, bool includesHeader) {

    if (includesHeader) {
        // Provided data includes the header, no need to make any extra room for it
        data.resize(bufferLength);

        memcpy(data.data(), buffer, bufferLength);

        // Do some checks that prevent reading past the end of the buffer
        if (getPayloadLength() + 12 > bufferLength) {
            throw std::runtime_error("Received fragment with invalid data length");
        }

        if (getFragmentStart() + getPayloadLength() > getPacketLength()) {
            throw std::runtime_error("Fragment does not fit in reported packet size.");
        }
    } else {
        // The data provided is only the payload, prepend some extra space for the header
        data.resize(bufferLength + 12);

        memcpy(data.data() + 12, buffer, bufferLength);
    }

}