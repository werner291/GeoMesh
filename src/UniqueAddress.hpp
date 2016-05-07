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

#ifndef GEOMESH_UNIQUEADDRESS_H
#define GEOMESH_UNIQUEADDRESS_H

#include <random>
#include <cstring>
#include <iomanip>
#include <netinet/in.h>
#include <sstream>
#include <iostream>

#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>

#include "Crypto.h"

const int ADDRESS_LENGTH_OCTETS = 16; // 128-bit addresses, we're optimistic.

/**
 * Short class / struct representing the XOR distance between two 128-bit addresses.
 */
struct AddressDistance {

    // Lower index means higher order.
    uint8_t data[ADDRESS_LENGTH_OCTETS];

    inline int compare(const AddressDistance& other) const {
	    return memcmp(data,other.data, ADDRESS_LENGTH_OCTETS);
    }

    inline bool operator<(const AddressDistance &other) const {
	    return compare(other) < 0;
    }

    inline bool operator>(const AddressDistance &other) const {
	    return compare(other) > 0;
    }

    /**
     * Whether this distance exactly matches the other, byte for byte.
     */
    inline bool operator==(const AddressDistance &other) const {
	    return compare(other) == 0;
    }

    inline bool operator <=(const AddressDistance &other) const {
        return *this < other || *this == other;
    }

    int getDifferingBits() {
        int count = 0;
        for (uint8_t i : data) {
            while (i != 0) { // Keep at it until the number reaches 0.
                if ((i & 1) == 1) {
                    count += 1;
                }
                i = i >> 1;
            }
        }
        return count;
    }

    int getMatchingPrefixLength() {
	    int match = 0;
	    
	    for (uint8_t i : data ) {
	        match += 8;
		if (i != 0) {
		while (i != 0) {
			i = i >> 1;
			match -= 1;
		}
		return match;
		}
	    }

	    return match;
    }

};

/**
 * Represents a 128-bit GeoMesh address.
 */
class Address {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

public:

    // Required for std::map lookups.

    inline int compare(const Address &other) const {
        return memcmp(bytes,other.bytes,ADDRESS_LENGTH_OCTETS);
    }

    inline bool operator<(const Address &other) const { return compare(other) < 0; }

    inline bool operator<=(const Address &other) const { return compare(other) <= 0; }

    inline bool operator>(const Address &other) const { return compare(other) > 0; }

    inline bool operator>=(const Address &other) const { return compare(other) >= 0; }


    /**
     * @return Whether this address exactly matches the other address.
     */
    bool operator==(const Address& other) const {
        return memcmp(bytes,other.bytes,ADDRESS_LENGTH_OCTETS) == 0;
    }

    inline bool operator!=(const Address& other) const { return ! this->operator==(other); }

    static Address generateRandom();

    static Address generateFromKey(const AddressPublicKey& keys);

    static Address fromString(const std::string& str);

    const std::string toString() const {

        std::stringstream ss;

        ss << std::hex << std::setfill('0');

        for (int i = 0; i + 1 < ADDRESS_LENGTH_OCTETS; i += 2) {

            if (i != 0) ss << ":";
            // TODO enable leading 0's
            ss << std::setw(2) << (unsigned int) bytes[i] << std::setw(2) << (unsigned int) bytes[i + 1];

        }

        return ss.str();
    }

    const std::string toBitString() const {
        std::stringstream str;
        for (int bit = 0; bit < ADDRESS_LENGTH_OCTETS * 8; ++bit) {
            str << (getBit(bit) ? '1' : '0');
        }
        return str.str();
    }

    static Address fromBytes(const uint8_t* bytes) {
        Address addr;

        memcpy(addr.bytes, bytes, ADDRESS_LENGTH_OCTETS);

        return addr;
    }

    void writeToSocketAddress(struct sockaddr_in6& socketAddress) const;

    void setBytes(const uint8_t *newBytes) {
        memcpy(bytes, newBytes, ADDRESS_LENGTH_OCTETS);
    }

    const uint8_t *getBytes() const {
        return bytes;
    }

    AddressDistance xorDistanceTo(const Address &other) const {

        AddressDistance dist;

        // Iterate over all the octets
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; ++i) {

            // Take the XOR distance.
            reinterpret_cast<uint8_t*>(dist.data)[i] = (bytes[i] ^ other.bytes[i]);

        }

        return dist;
    }

    inline int getBit(const int& position) const {

        uint8_t byte = bytes[position / 8];

        return (byte >> (7 - (position % 8))) & 1;
    }
};

#endif //GEOMESH_UNIQUEADDRESS_H
