//
// Created by System Administrator on 3/1/16.
//

#include <algorithm>
#include <gtest/gtest.h>
#include "../src/Packet.h"
#include "../src/geomeshd/UDPManager.h"

TEST(udpbridge,fragmentation) {

    uint8_t data[MAX_PACKET_SIZE-1];

    for (int i=0; i< MAX_PACKET_SIZE-1; ++i) {
        data[i] = i % 256;
    }

    PacketPtr packet = Packet::createFromData(data, MAX_PACKET_SIZE-1);

    auto fragments = UDPManager::fragmentPacket(packet, 375, 7);

    std::random_shuffle(fragments.begin(),fragments.end());

    UDPReceptionBuffer buffer;

    for (int i = fragments.size()-1; i >=0; --i) {
        buffer.receive(fragments[i]);
    }

    buffer.receive(fragments[1]);

    bool packetAvaliable = buffer.isFullPacketAvailable();

    EXPECT_TRUE(packetAvaliable);

    PacketPtr reconstructed = buffer.getReconstructedPacket();

    for (int i=0; i< MAX_PACKET_SIZE-1; ++i) {
        EXPECT_EQ(i % 256, reconstructed->getData()[i]);
    }

}

TEST(udpbridge,fragmentation_location) {

    Location loc(-5,60);
    Address address = Address::generateRandom();

    PacketPtr packet = Packet::createLocationInfoPacket(loc, address);

    auto fragments = UDPManager::fragmentPacket(packet, 375, 7);

    std::random_shuffle(fragments.begin(),fragments.end());

    UDPReceptionBuffer buffer;

    for (int i = fragments.size()-1; i >=0; --i) {
        buffer.receive(fragments[i]);
    }

    bool packetAvaliable = buffer.isFullPacketAvailable();

    EXPECT_TRUE(packetAvaliable);

    PacketPtr reconstructed = buffer.getReconstructedPacket();

    //EXPECT_EQ(loc, reconstructed->getSourceLocation());
    EXPECT_EQ(address, reconstructed->getSourceAddress());
}