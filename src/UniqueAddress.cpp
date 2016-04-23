//
// Created by Werner Kroneman on 28-01-16.
//

#include <netinet/in.h>
#include <arpa/inet.h>
#include "UniqueAddress.hpp"

std::random_device rdev;
std::mt19937 rgen(rdev());
std::uniform_int_distribution<unsigned char> addrgen(0,255);


Address Address::generateRandom() { // TODO use keypair for generating addresses.
    Address blank;

    // Unallocated Unique Local Address range for IPv6 with prefix fc::/8
    blank.bytes[0] = 0xfc;
    // The "f4" octet is to distinguis from CJDNS addresses. GeoMesh doesn't claim the whole range. It doesn't claim
    // anything really, but this should make things easier when there are conflicts with other uniquely generated
    // addresses.
    blank.bytes[1] = 0xf4;

    // Fill the rest with random bytes. Use the full 128 bits.
    for (int i=2; i<ADDRESS_LENGTH_OCTETS;i++)
        blank.bytes[i] = addrgen(rgen);

    return blank;
}

Address Address::generateFromKeys(const KeyPair& keys) {

    std::vector<uint8_t> pubkey = keys.getPublicKeyAsBytes();

    
    auto firstHash = simpleSHA256(pubkey.data(),pubkey.size());

    auto secondHash = simpleSHA256(firstHash.data(), firstHash.size());
   
    uint8_t addrBytes[ADDRESS_LENGTH_OCTETS];

    addrBytes[0] = 0xfc;
    addrBytes[1] = 0xf4;

    memcpy(addrBytes + 2, secondHash.data(), ADDRESS_LENGTH_OCTETS - 2);

    return Address::fromBytes(addrBytes);

}

Address Address::fromString(const std::string& str) {
    struct in6_addr ipv6data;
    inet_pton(AF_INET6, str.c_str(), &ipv6data);

    Address blank;

    std::memcpy(blank.bytes, ipv6data.s6_addr, ADDRESS_LENGTH_OCTETS);

    return blank;
}

void Address::writeToSocketAddress(struct sockaddr_in6& socketAddress) const {

    std::memcpy(&socketAddress.sin6_addr.s6_addr, bytes, ADDRESS_LENGTH_OCTETS);



}
