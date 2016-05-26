//
// Created by System Administrator on 2/8/16.
//

#include <gtest/gtest.h>
#include <cstring>
#include "../src/UniqueAddress.hpp"
#include "../src/Packet.hpp"

const double SMALL = 0.00001;

TEST(location_info, location_info_checksum) {

    GPSLocation loc(5, 90);
    Address addr = Address::generateRandom();

    PacketPtr pack = Packet::createLocationInfoPacket(loc, addr);

    EXPECT_EQ(pack->verifyLocationInformation(), true);

    GPSLocation packetLoc = pack->getSourceLocation();

    EXPECT_NEAR(packetLoc.lat, loc.lat, SMALL);
    EXPECT_NEAR(packetLoc.lon, loc.lon, SMALL);

    EXPECT_EQ(pack->getLocationInfoHopCount(), 1);

    EXPECT_EQ(pack->getSourceAddress(), addr);

    pack->getHeader()[5] -= 1; // Change one of the bytes by 1.

    EXPECT_EQ(pack->verifyLocationInformation(), false);

}
