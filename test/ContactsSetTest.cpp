//
// Created by Werner Kroneman on 04-04-16.
//

#include <set>
#include "../src/ContactsSet.hpp"
#include "../src/UniqueAddress.hpp"
#include <gtest/gtest.h>

TEST(ContactsSetTest, insert_order_duplicates) {

    std::set<Address> addresses;
    ContactsSet contacts;

    for (int i = 0; i < 500; ++i) {
        addresses.insert(Address::generateRandom());

    }

    for (const Address& addr : addresses) {
        auto itr = contacts.insert(ContactsSet::Entry(addr,Location(0,0),0));

        EXPECT_EQ(addr, itr->address);
    }

    for (int i = 0; i < 50; i++) {
    	
    // Add an address twice to see if it correctly prevents duplicates
    Address duplicate = Address::generateRandom();
    addresses.insert(duplicate);
    contacts.insert(ContactsSet::Entry(duplicate,Location(0,0),0));
    contacts.insert(ContactsSet::Entry(duplicate,Location(0,0),0));

    }

    Address lastAddr;
    bool first = true;

    for (const ContactsSet::Entry& entry : contacts) {

        if (!first) {
            EXPECT_GE(entry.address, lastAddr);
            EXPECT_GE(entry.address.toBitString(), lastAddr.toBitString());
        }
        lastAddr = entry.address;
        first = false;
    }

    for (const Address& addr : addresses) {
        auto itr = contacts.findClosestEntry(addr);

        EXPECT_EQ(addr, itr->address);
    }

    for (const ContactsSet::Entry& entry : contacts) {

        EXPECT_EQ(1, addresses.erase(entry.address));
    }

    EXPECT_TRUE(addresses.empty());
}

TEST(ContactsSetTest, closest_not_equal_single_entry) {

    ContactsSet contacts;

    Address testAddr = Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555");

    contacts.insert(ContactsSet::Entry(testAddr, Location(0,0), 0));

    auto itr = contacts.findClosestEntry(Address::fromString("4444:4444:4444:4444:4444:4444:4444:4444"));

    EXPECT_NE(itr,contacts.end());

    EXPECT_EQ(testAddr, itr->address);
}

TEST(ContactsSetTest, closest_not_equal_single_entry_part_equal) {

    ContactsSet contacts;

    Address testAddr = Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555");

    contacts.insert(ContactsSet::Entry(testAddr, Location(0,0), 0));

    auto itr = contacts.findClosestEntry(Address::fromString("4444:5555:4444:4444:4444:4444:4444:4444"));

    EXPECT_NE(itr,contacts.end());

    EXPECT_EQ(testAddr, itr->address);
}
