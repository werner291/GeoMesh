//
// Created by System Administrator on 2/12/16.
//

#include <gtest/gtest.h>
#include "../src/UniqueAddress.h"

TEST(distance_xor_compute, identity_random) {

    Address a = Address::generateRandom();

    AddressDistance d = a.xorDistanceTo(a);

    EXPECT_EQ(d.data[0],0);
    EXPECT_EQ(d.data[1],0);
    EXPECT_EQ(d.data[2],0);
    EXPECT_EQ(d.data[3],0);
}

TEST(distance_xor_compute, extremes) {

    Address a = Address::fromString("0000:0000:0000:0000:0000:0000:0000:0000");

    Address b = Address::fromString("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF");

    AddressDistance d = a.xorDistanceTo(b);

    EXPECT_EQ(d.data[0],UINT32_MAX);
    EXPECT_EQ(d.data[1],UINT32_MAX);
    EXPECT_EQ(d.data[2],UINT32_MAX);
    EXPECT_EQ(d.data[3],UINT32_MAX);
}

TEST(distance_xor_compute, extremes_order) {

    Address a = Address::fromString("0000:0000:0000:0000:0000:0000:0000:0000");

    Address b = Address::fromString("FFFF:FFFF:FFFF:FFFF:0000:0000:0000:0000");

    AddressDistance d = a.xorDistanceTo(b);

    EXPECT_EQ(d.data[0],UINT32_MAX);
    EXPECT_EQ(d.data[1],UINT32_MAX);
    EXPECT_EQ(d.data[2],0);
    EXPECT_EQ(d.data[3],0);
}

TEST(distance_slot, zero) {

    AddressDistance dist;

    dist.data[0] = 0;
    dist.data[1] = 0;
    dist.data[2] = 0;
    dist.data[3] = 0;

    EXPECT_EQ(dist.getDHTSlotNumber(), 0);

}

TEST(distance_slot, one) {

    AddressDistance dist;

    dist.data[0] = 0;
    dist.data[1] = 0;
    dist.data[2] = 0;
    dist.data[3] = 1;

    EXPECT_EQ(dist.getDHTSlotNumber(), 1);

}

TEST(distance_slot, max_matched) {

    AddressDistance dist;

    dist.data[0] = 0x80000000;
    dist.data[1] = 0;
    dist.data[2] = 0;
    dist.data[3] = 0;

    EXPECT_EQ(dist.getDHTSlotNumber(), 128);

}

TEST(distance_slot, max_just_over_same_int) {

    AddressDistance dist;

    dist.data[0] = 0x80000001;
    dist.data[1] = 0;
    dist.data[2] = 0;
    dist.data[3] = 0;

    EXPECT_EQ(dist.getDHTSlotNumber(), 128);

}

TEST(distance_slot, max_just_over_different_int) {

    AddressDistance dist;

    dist.data[0] = 0x80000000;
    dist.data[1] = 0;
    dist.data[2] = 0;
    dist.data[3] = 1;

    EXPECT_EQ(dist.getDHTSlotNumber(), 128);

}