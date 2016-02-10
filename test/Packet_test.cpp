//
// Created by System Administrator on 2/8/16.
//

#include <gtest/gtest.h>
#include <cstring>
#include "../src/UniqueAddress.h"
#include "../src/Packet.h"

TEST(packet_creation_from_ipv6, from_ipv6
) {

Address testAddrSource = Address::generateRandom();
Address testAddrDestination = Address::generateRandom();

unsigned char ipv6Packet[1500];

std::memcpy(ipv6packet
+ 8, testAddrSource.

getBytes(), ADDRESS_LENGTH_OCTETS

);


PacketPtr testPacket = Packet::createFromIPv6();


}

TEST(location_info, location_info_checksum
) {

Location loc(5, 90);
Address addr = Address::generateRandom();

PacketPtr pack = Packet::createLocationInfoPacket(loc, addr);

EXPECT_EQUAL(pack
->

verifyLocationInformation(),

true);

pack->

getBytes()[5]

-= 1; // Change one of the bytes by 1.

EXPECT_EQUAL(pack
->

verifyLocationInformation(),

false);

}