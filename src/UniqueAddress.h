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

const int ADDRESS_LENGTH_OCTETS = 16; // 128-bit addresses, we're optimistic.

struct AddressDistance {

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

    inline bool operator==(const AddressDistance &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / 4; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

};

class Address {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

public:

    /**
     * @return Whether this address exactly matches the other address.
     */
    bool operator==(const Address& other) const {
        return memcmp(bytes,other.bytes,ADDRESS_LENGTH_OCTETS) == 0;
    }

    static Address generateRandom();

    static Address fromString(std::string str);

    std::string toString() {

        std::stringstream ss;

        ss << std::hex << std::setfill('0');

        for (int i = 0; i + 1 < ADDRESS_LENGTH_OCTETS; i += 2) {

            if (i != 0) ss << ":";
            // TODO enable leading 0's
            ss << std::setw(2) << (unsigned int) bytes[i] << std::setw(2) << (unsigned int) bytes[i + 1];

        }

        return ss.str();
    }

    void writeToSocketAddress(struct sockaddr_in6& socketAddress) const;

    void setBytes(uint8_t *newBytes) {
        memcpy(bytes, newBytes, ADDRESS_LENGTH_OCTETS);
    }

    const uint8_t *getBytes() const {
        return bytes;
    }

    AddressDistance xorDistanceTo(const Address &other) const {

        AddressDistance dist;

        // Iterate over all the octets
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; ++i) {

            // Take the XOR distance, multiply according to the octet position.
            dist.data[i] = (bytes[i] ^ other.bytes[i]);

        }

        return dist;
    }
};

#endif //GEOMESH_UNIQUEADDRESS_H
