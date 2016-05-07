/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PacketFragment.hpp"

#include <exception>

#include "Logger.hpp"

PacketFragment::PacketFragment(const uint8_t* buffer, int bufferLength, bool includesHeader) {

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
