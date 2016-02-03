//
// Created by Werner Kroneman on 28-01-16.
//

#ifndef GEOMESH_UNIQUEADDRESS_H
#define GEOMESH_UNIQUEADDRESS_H

#include <random>
#include <netinet/in.h>
#include "constants.h"

struct Address {
    unsigned char bytes[ADDRESS_LENGTH_OCTETS];

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
};

#include "Location.h"
#include "AbstractInterface.h"
#include "constants.h"

#endif //GEOMESH_UNIQUEADDRESS_H
