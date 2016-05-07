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

#include <netinet/in.h>
#include <arpa/inet.h>
#include "UniqueAddress.hpp"

std::random_device rdev;
std::mt19937 rgen(rdev());
std::uniform_int_distribution<unsigned char> addrgen(0,255);

using namespace CryptoPP;

Address Address::generateRandom() { 
    Address blank;

    // Unallocated Unique Local Address range for IPv6 with prefix fc::/8
    blank.bytes[0] = 0xfc;
    // The "f4" octet is to distinguish from CJDNS addresses. GeoMesh doesn't
    // claim the whole range. It doesn't claim anything really, but this should
    //make things easier when there are conflicts with other uniquely generated
    // addresses.
    blank.bytes[1] = 0xf4;

    // Fill the rest with random bytes. Use the full 128 bits.
    for (int i=2; i<ADDRESS_LENGTH_OCTETS;i++)
        blank.bytes[i] = addrgen(rgen);

    return blank;
}

Address Address::generateFromKey(const AddressPublicKey& key) {

    // TODO check whether this is big enough!
    uint8_t buffer[2000];
 
    ArraySink sink(buffer,2000);
    // TODO check whether this is safe!
    // Does this polymorphically save the private key too?
    key.DEREncodePublicKey(sink); 

    byte addrBytes[2 + SHA256::DIGESTSIZE];

    addrBytes[0] = 0xfc;
    addrBytes[1] = 0xf4;

    // TODO check whether TotalPutLength is the right method to call
    SHA256().CalculateDigest(addrBytes + 2, buffer, sink.TotalPutLength());

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

    std::memcpy(&socketAddress.sin6_addr.s6_addr, bytes, 
                ADDRESS_LENGTH_OCTETS);

}
