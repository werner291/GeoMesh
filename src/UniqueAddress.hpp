//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UNIQUEADDRESS_H
#define GEOMESH_UNIQUEADDRESS_H

#include <random>
#include <cstring>
#include <iomanip>
#include <netinet/in.h>
#include <sstream>
#include <iostream>

const int ADDRESS_LENGTH_OCTETS = 16; // 128-bit addresses, we're optimistic.

/**
 * Short class / struct representing the XOR distance between two 128-bit addresses.
 */
struct AddressDistance {

    // Lower index means higher order.
    uint32_t data[ADDRESS_LENGTH_OCTETS / 4];

    inline bool operator<(const AddressDistance &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / 4; ++i) {
            if (data[i] < other.data[i]) return true;
        }
        return false;
    }

    inline bool operator>(const AddressDistance &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / 4; ++i) {
            if (data[i] > other.data[i]) return true;
        }
        return false;
    }

    /**
     * Whether this distance exactly matches the other, byte for byte.
     */
    inline bool operator==(const AddressDistance &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / 4; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    inline bool operator <=(const AddressDistance &other) const {
        return *this < other || *this == other;
    }

};

/**
 * Represents a 128-bit GeoMesh address.
 */
class Address {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

public:

    // Required for std::map lookups.

    inline bool operator<(const Address &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; ++i) {
            if (bytes[i] != other.bytes[i]) {
                return bytes[i] < other.bytes[i];
            }
        }
        return false;
    }

    inline bool operator>(const Address &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; ++i) {
            if (bytes[i] != other.bytes[i]) {
                return bytes[i] > other.bytes[i];
            }
        }
        return false;
    }

    /**
     * @return Whether this address exactly matches the other address.
     */
    bool operator==(const Address& other) const {
        return memcmp(bytes,other.bytes,ADDRESS_LENGTH_OCTETS) == 0;
    }

    inline bool operator!=(const Address& other) const { return ! this->operator==(other); }

    static Address generateRandom();

    static Address fromString(std::string str);

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
};

#endif //GEOMESH_UNIQUEADDRESS_H