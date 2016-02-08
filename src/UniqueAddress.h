//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UNIQUEADDRESS_H
#define GEOMESH_UNIQUEADDRESS_H

#include <random>
#include <cstring>
#include <netinet/in.h>
#include "constants.h"

struct Address {

    uint8_t bytes[ADDRESS_LENGTH_OCTETS];

    bool operator==(const Address& other) const {
        return memcmp(bytes,other.bytes,ADDRESS_LENGTH_OCTETS) == 0;
    }

    bool isDestinationOf(const DataBufferPtr& packet) const {

        char* packetAddrPtr = packet->data() + DESTINATION_ADDRESS;

        return memcmp(bytes,packetAddrPtr,ADDRESS_LENGTH_OCTETS) == 0;
    }

    void writeAsDestination(DataBufferPtr& packet) const {
        memcpy(packet->data() + DESTINATION_ADDRESS, bytes,ADDRESS_LENGTH_OCTETS);
    }

    static Address generateRandom();

    static Address fromString(std::string str);

    void writeToSocketAddress(struct sockaddr_in6& socketAddress) const;

    double xorDistanceTo(const Address &other) {
        // Distance between the wto adresses
        double dist = 0;

        // By how much to multiply the XOR-ed octets
        double multiplier = 1;

        // By how much to multiply the multiplier each iteration
        double iterationMult = pow(2, 8);

        // Iterate over all the octets
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; ++i) {

            // Take the XOR distance, multiply according to the octet position.
            dist += ((double) (bytes[i] ^ other.bytes[i])) * multiplier;

            // Multiply the multiplier.
            multiplier *= iterationMult;

        }

        return dist;
    }
};

#include "Location.h"
#include "AbstractInterface.h"
#include "constants.h"

#endif //GEOMESH_UNIQUEADDRESS_H
