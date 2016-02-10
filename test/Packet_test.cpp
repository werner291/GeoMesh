//
// Created by System Administrator on 2/8/16.
//

#include <gtest/gtest.h>
#include <cstring>
#include "../src/UniqueAddress.h"
#include "../src/Packet.h"

TEST(packet_creation_from_ipv6, from_ipv6) {

    Address testAddrSource = Address::generateRandom();
    Address testAddrDestination = Address::generateRandom();

    uint8_t ipv6Packet[1280];

    memset(ipv6Packet, 1, 1280);

    PacketPtr testPacket = Packet::createFromIPv6(ipv6Packet, 1280, Location(-1, -1), Location(1, 1));

    EXPECT_EQ(testPacket->isDestination(testAddrDestination), true);


}

const double SMALL = 0.00001;

TEST(location_info, location_info_checksum) {

    Location loc(5, 90);
    Address addr = Address::generateRandom();

    PacketPtr pack = Packet::createLocationInfoPacket(loc, addr);

    EXPECT_EQ(pack->verifyLocationInformation(), true);

    Location packetLoc = pack->getSourceLocation();

    EXPECT_NEAR(packetLoc.lat, loc.lat, SMALL);
    EXPECT_NEAR(packetLoc.lon, loc.lon, SMALL);

    EXPECT_EQ(pack->getLocationInfoHopCount(), 1);

    EXPECT_EQ(pack->getSourceAddress(), addr);

    pack->getHeader()[5] -= 1; // Change one of the bytes by 1.

    EXPECT_EQ(pack->verifyLocationInformation(), false);

}