//
// Created by System Administrator on 3/1/16.
//

#include <algorithm>
#include <gtest/gtest.h>
#include "../src/Packet.hpp"
#include "../src/geomeshd/UDPManager.hpp"

TEST(udpbridge,fragmentation) {



    uint8_t data[MAX_PACKET_SIZE-1];

    for (int i=0; i< MAX_PACKET_SIZE-1; ++i) {
        data[i] = i % 256;
    }

    PacketPtr packet = std::make_shared<Packet>(data, MAX_PACKET_SIZE-1);


    std::vector<PacketFragmentPtr> fragments;
    
    FragmentingLinkEndpoint sender([&](const PacketFragmentPtr frag, uint16_t iFaceID) {
        EXPECT_EQ(sender.getInterfaceId(), iFaceID);
        fragments.push_back(frag);
    });

    std::random_shuffle(fragments.begin(), fragments.end());

    PacketDefragmenter buffer;

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

