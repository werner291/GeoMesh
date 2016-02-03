//
// Created by Werner Kroneman on 28-01-16.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include "UniqueAddress.h"

std::random_device rdev;
std::mt19937 rgen(rdev());
std::uniform_int_distribution<unsigned char> addrgen(0,255);

Address Address::generateRandom() { // TODO use keypair for generating addresses.
    Address blank;


    blank.bytes[0] = 0xfc; // Unallocated Unique Local Address range for IPv6 with prefix fc::/8

    for (int i=1; i<ADDRESS_LENGTH_OCTETS;i++)
        blank.bytes[i] = addrgen(rgen);

    return blank;
}

Address Address::fromString(std::string str) {
    struct in6_addr ipv6data;
    inet_pton(AF_INET6, str.c_str(), &ipv6data);

    Address blank;

    std::memcpy(blank.bytes, ipv6data.s6_addr, ADDRESS_LENGTH_OCTETS);

    return blank;
}

void Address::writeToSocketAddress(struct sockaddr_in6& socketAddress) const {

    std::memcpy(&socketAddress.sin6_addr.__u6_addr.__u6_addr8, bytes, ADDRESS_LENGTH_OCTETS);



}