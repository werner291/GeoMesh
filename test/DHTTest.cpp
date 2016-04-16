//
// Created by System Administrator on 3/18/16.
//

#include "../src/LocationLookupManager.hpp"
#include "../src/LocalInterface.hpp"
#include <gtest/gtest.h>
#include <iostream>

const std::function<bool(PacketPtr)> EMPTY_SEND_STRATEGY = [](PacketPtr packet) -> bool { return false; };

class DHTSimplifiedNode {

public:
    Address addr;
    VirtualLocationManager vlm;
    LocalPacketHandler lh;
    LocationLookupManager llm;
    LocalInterface li;

    DHTSimplifiedNode(const Address &addr,
                      const Location &loc,
                      std::function<bool(PacketPtr)> sendPacket = EMPTY_SEND_STRATEGY)
            : addr(addr), vlm(loc), lh(vlm, addr, sendPacket), llm(lh, addr, vlm), li(lh, llm) { }
};

/**
 * Test of whether a LocationLookupManager with pre-configured contacts forwards
 * FIND_CLOSEST messages correctly.
 */
TEST(DHTTest, relay_find_preconfigured) {

    // Address/location of the simulated node.
    Address simulatedAddress = Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555");
    VirtualLocationManager simulatedVLM(Location(0, 0));

    // Initialize local handler with empty send strategy.
    LocalPacketHandler localHandler(simulatedVLM, simulatedAddress, EMPTY_SEND_STRATEGY);

    // The LLM of the simulated node
    LocationLookupManager llm(localHandler, simulatedAddress, simulatedVLM);

    // Give it a contact very close to its own position
    llm.processEntrySuggestion(Address::fromString("5555:5555:5555:5555:5555:5555:5555:5551"),
                               Location(2, 5), 0);

    // And many random contacts
    for (int i = 0; i < 5000; ++i) {
        llm.processEntrySuggestion(Address::generateRandom(), Location(0, 0), 0);
    }

    /*
     * Set strategy to result checker.
     * Packet should be bound for the contact that is really close to the simulated node.
     */
    localHandler.setSendPacketStrategy([&](PacketPtr packet) -> bool {

        EXPECT_EQ(Address::fromString("5555:5555:5555:5555:5555:5555:5555:5551"),
                  packet->getDestinationAddress());

        Location destLoc = packet->getDestinationLocation();

        EXPECT_NEAR(2, destLoc.lat, 0.01);
        EXPECT_NEAR(5, destLoc.lon, 0.01);

        // Check whether the source is correctly set to the simulated node.
        EXPECT_EQ(simulatedAddress, packet->getSourceAddress());
        EXPECT_EQ(simulatedVLM.getLocation(), packet->getSourceLocation());

        return true;
    });

    // Create a FIND_CLOSEST message bound for the simulated node from some random source
    PacketPtr packet = std::make_shared<Packet>(
            Address::generateRandom(),
            Location(90, 60),
            simulatedAddress,
            simulatedVLM.getLocation(),
            MSGTYPE_DHT_FIND_CLOSEST,
            FIND_CLOSEST_MESSAGE_SIZE
    );

    // Write the lookup message, original requester is random, query is the contact that is really close
    LocationLookupManager::writeLookupMessage(packet->getPayload(),
                                              Address::generateRandom(),
                                              Location(0, 0),
                                              Address::fromString("5555:5555:5555:5555:5555:5555:5555:5551"));

    // Simulate handling and reception by LLM.
    localHandler.handleLocalPacket(packet);

    // Create another packet, again random sender and bound for simulated node.
    PacketPtr packet2 = std::make_shared<Packet>(
            Address::generateRandom(),
            Location(90, 60),
            simulatedAddress,
            simulatedVLM.getLocation(),
            MSGTYPE_DHT_FIND_CLOSEST,
            FIND_CLOSEST_MESSAGE_SIZE
    );

    // Similar to first, but query is close to a specific contact without matching exactly.
    // Send strategy not modified since same output is expected.
    LocationLookupManager::writeLookupMessage(packet->getPayload(),
                                              Address::generateRandom(),
                                              Location(0, 0),
                                              Address::fromString("5555:5555:5555:5555:5555:5555:5555:5541"));

    // Simulate reception and handling.
    localHandler.handleLocalPacket(packet2);
}

TEST(dhtTest, respond_find) {

    int a = 9;
    DHTSimplifiedNode node(Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555"), Location(10, -56));

    int sent = 0;

    node.lh.setSendPacketStrategy([&](PacketPtr packet) -> bool {

        EXPECT_EQ(MSGTYPE_DHT_FIND_RESPONSE, packet->getMessageType());

        sent += 1;

        return true;
    });

    // Create a FIND_CLOSEST message bound for the simulated node from some random source
    PacketPtr packet = std::make_shared<Packet>(
            Address::generateRandom(),
            Location(90, 60),
            node.addr,
            node.vlm.getLocation(),
            MSGTYPE_DHT_FIND_CLOSEST,
            FIND_CLOSEST_MESSAGE_SIZE
    );

    // Write the lookup message, original requester is random, query exactly the node
    LocationLookupManager::writeLookupMessage(packet->getPayload(),
                                              Address::generateRandom(),
                                              Location(0, 0),
                                              Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555"));

    node.lh.handleLocalPacket(packet);

    EXPECT_EQ(1, sent);

}

TEST(dhtTest, refresh_procedure) {

    DHTSimplifiedNode node(Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555"), Location(0, 0));

    // And many random contacts
    for (int i = 0; i < 50; ++i) {
        node.llm.processEntrySuggestion(Address::generateRandom(), Location(0, 0), 0);
    }

    int count = 0;

    node.lh.setSendPacketStrategy([&](PacketPtr packet) -> bool {

        count += 1;

        Address query = Address::fromBytes(packet->getPayload() + FIND_CLOSEST_QUERY);

        AddressDistance xorDist = node.addr.xorDistanceTo(query);

        int differingBits = xorDist.getDifferingBits();

        EXPECT_EQ(1, differingBits);

        return true;
    });

    node.llm.refreshRoutingTable();

    EXPECT_EQ(ADDRESS_LENGTH_OCTETS * 8, count);
}

TEST(dhtTest, local_minimum_response) {

    DHTSimplifiedNode node(Address::fromString("5555:5555:5555:5555:5555:5555:5555:5555"), Location(0, 0));

    // And many random contacts
    for (int i = 0; i < 50; ++i) {
        node.llm.processEntrySuggestion(Address::generateRandom(), Location(0, 0), 0);
    }

    int sent = 0;

    node.lh.setSendPacketStrategy([&](PacketPtr packet) -> bool {

        EXPECT_EQ(MSGTYPE_DHT_FIND_RESPONSE, packet->getMessageType());

        sent += 1;

        return true;
    });

    // Create a FIND_CLOSEST message bound for the simulated node from some random source
    PacketPtr packet = std::make_shared<Packet>(
            Address::generateRandom(),
            Location(90, 60),
            node.addr,
            node.vlm.getLocation(),
            MSGTYPE_DHT_FIND_CLOSEST,
            FIND_CLOSEST_MESSAGE_SIZE
    );

    // Write the lookup message, original requester is random, query exactly the node
    LocationLookupManager::writeLookupMessage(packet->getPayload(),
                                              Address::generateRandom(),
                                              Location(0, 0),
                                              Address::fromString("5555:5555:5555:5555:5555:5555:5555:5554"));

    node.lh.handleLocalPacket(packet);

    EXPECT_EQ(1, sent);
}

typedef std::vector<std::shared_ptr<DHTSimplifiedNode> >::iterator NodeItr;
void printNodeGraph(NodeItr begin, NodeItr end) {
    
std::cout << "digraph knows {" << std::endl;

    for (auto itr = begin; itr != end; ++itr) {
	for (const  ContactsSet::Entry& entry : (*itr)->llm.getContacts()) {
	    std::cout << "\"" << (*itr)->addr.toBitString().substr(16,8) 
		    << "\" -> \""<< entry.address.toBitString().substr(16,8) << "\"" << std::endl;
	}
    }
std::cout << "}" << std::endl;
}

/*
 * Test to see whether what is initially a chain of nodes
 * automatically reorganizes itself into a Kademlia-like
 * network, and whether sending messages from one node
 * to another is possible.
 */
TEST(DHTTest, routingTest) {

    const int MAX_NODES = 50;

    std::vector<std::shared_ptr<DHTSimplifiedNode>> nodes;
    std::map<Address, std::shared_ptr<DHTSimplifiedNode>> nodesByAddress;

    for (int i = 0; i < MAX_NODES; i++) {
        std::shared_ptr<DHTSimplifiedNode> node = std::make_shared<DHTSimplifiedNode>(
                Address::generateRandom(), Location(0, 0),
                [&](const PacketPtr &packet) -> bool {

	            if (packet->getMessageType() == MSGTYPE_DHT_FIND_CLOSEST) {

		         Address query = Address::fromBytes(packet->getPayload() + FIND_CLOSEST_QUERY); 
			 int matching = query.xorDistanceTo(packet->getDestinationAddress()).getMatchingPrefixLength();
			 //std::cout << "Forwarding query for " << query.toString() << 
			   //           " to " << packet->getDestinationAddress().toString() << 
			     //         " Matching: " << matching <<  std::endl;
		    }

                    Address destination = packet->getDestinationAddress();

                    auto itr = nodesByAddress.find(destination);

                    EXPECT_NE(nodesByAddress.end(), itr);
                    if (nodesByAddress.end() == itr) {
                        std::cout << "Sending to unknown address" <<
                        destination.toString() << std::endl;
                        return false;
                    }

                    itr->second->lh.handleLocalPacket(packet);
                    return true;
                });

        nodes.push_back(node);
        nodesByAddress[node->addr] = node;
    }

    /*
     * Send hello messages between pairs of nodes to form a chain.
     * This should be enough information to start forming a network.
     */

    for (int i = 1; i < nodes.size(); i++) {
        // Add nodes[i-1] to the contacts list of nodes[i], this will serve as the
        // bootstrap node for nodes[i].
        nodes[i]->llm.processEntrySuggestion(nodes[i - 1]->addr, Location(0, 0), time(nullptr) + 500);
    }

    nodes.front()->llm.processEntrySuggestion(nodes.back()->addr, Location(0, 0), time(nullptr) + 500);


    for (int i = 0; i < 10; ++i) {
	    printNodeGraph(nodes.begin(),nodes.end());
        for (std::shared_ptr<DHTSimplifiedNode> node : nodes) {
            node->llm.refreshRoutingTable();
        }
    }

    const int TESTS = 1;
    for (int t=0;t<TESTS;++t) {

	    int a = rand() % nodes.size();
		    int b = rand() % nodes.size();
	    auto origin = nodes[a];
            auto target = nodes[b];

	    std::cout << "Testing location lookup of " << target->addr.toString() 
		    << " by " << origin->addr.toString() << " (initial distance " << abs(a-b) << ")" << std::endl;
    Address addr = target->addr;
    Location expected = target->vlm.getLocation();

    bool received = false;

    origin->llm.addListener(
            [&](const Address &address, const Location &loc, time_t expires) {
                if (address == addr) {
                    if (loc == expected) {
                        std::cout << "Correct location lookup response!" << std::endl;
                        received = true;
                    } else {
                        std::cout << "Received response to request with wrong location!" << std::endl;
                    }
                } else {
                    std::cout << "Ok, but not what we asked for..." << std::endl;
                    std::cout << "Received response from: " << address.toString() << std::endl;

		    for (const ContactsSet::Entry& entry : nodesByAddress[address]->llm.getContacts()) {
		        std::cout << entry.address.toString() << " - " << addr.xorDistanceTo(address).getMatchingPrefixLength() << std::endl;
		    }
                }
            });


    Logger::setLogLevel(LogLevel::DEBUG);

    origin->llm.requestLocationLookup(addr);
    EXPECT_TRUE(received);
    
    origin->llm.removeAllListeners();
    }
}


