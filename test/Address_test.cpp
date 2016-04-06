//
// Created by System Administrator on 2/12/16.
//

#include <gtest/gtest.h>
#include "../src/UniqueAddress.hpp"

TEST(address_base, equality) {

    Address a = Address::generateRandom();

    Address b = Address::fromBytes(a.getBytes());

    EXPECT_EQ(a,b);

    EXPECT_FALSE(a<b);
    EXPECT_FALSE(b<a);
}

TEST(address_base, equality_false) {

    Address a = Address::generateRandom();

    Address b = Address::generateRandom();

    EXPECT_FALSE(a == b);
}

TEST(address_base, inequality) {

    Address a = Address::fromString("aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa");

    Address b = Address::fromString("aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaab");

    EXPECT_LT(a,b);

    EXPECT_GT(b,a);
}

TEST(address_base, inequality_2) {

    Address a = Address::fromString("aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa");

    Address b = Address::fromString("aaaa:aaaa:aaaa:abaa:aaaa:aaaa:aaaa:aaaa");

    EXPECT_LT(a,b);

    EXPECT_GT(b,a);
}

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

    EXPECT_EQ(64, d.getDifferingBits());

    EXPECT_EQ(d.data[0],UINT32_MAX);
    EXPECT_EQ(d.data[1],UINT32_MAX);
    EXPECT_EQ(d.data[2],0);
    EXPECT_EQ(d.data[3],0);
}

TEST(address,map_of_addresses) {

    std::vector<Address> addresses;
    std::map<Address,int> addressesMap;

    for (int i = 0; i < 10; ++i) {
        addresses.push_back(Address::generateRandom());
        addressesMap[addresses[i]] = i;
        //EXPECT_EQ(i, addressesMap[addresses[i]]);
    }

    for (int i = 0; i < 10; ++i) {
        auto itr = addressesMap.find(addresses[i]);



        EXPECT_FALSE(itr == addressesMap.end());
        EXPECT_EQ(i, itr->second);
    }



}

TEST(address, getBit) {

    Address addr = Address::fromString("0000:0000:0002:0001:0000:0000:0002:0001");

    EXPECT_EQ(1, addr.getBit(127));
    EXPECT_EQ(0, addr.getBit(111));
    EXPECT_EQ(1, addr.getBit(110));

}