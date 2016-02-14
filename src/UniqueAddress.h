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

    inline bool operator==(const AddressDistance &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / 4; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    inline int getDHTSlotNumber() const {

        for (int index=0; index < ADDRESS_LENGTH_OCTETS / 4; ++index) {
            if (data[index] != 0) {

                uint32_t number = data[index];

                int bitPos = 0;

                while (number != 0) {
                    bitPos++;
                    number /= 2; //  How many times can we divide by two until the numbe hits 0? (This is integer division.)
                }

                return ((ADDRESS_LENGTH_OCTETS / 4 - 1) - index) * 32 + bitPos;

            }
        }

        return 0;

    }

};

class Address {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

public:

    // Required for std::map lookups.

    inline bool operator<(const Address &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / sizeof(uint8_t); ++i) {
            if (bytes[i] < other.bytes[i]) return true;
        }
        return false;
    }

    inline bool operator>(const Address &other) const {
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS / sizeof(uint8_t); ++i) {
            if (bytes[i] > other.bytes[i]) return true;
        }
        return false;
    }

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
            reinterpret_cast<uint8_t*>(dist.data)[i] = (bytes[i] ^ other.bytes[i]);

        }

        return dist;
    }
};

#endif //GEOMESH_UNIQUEADDRESS_H
