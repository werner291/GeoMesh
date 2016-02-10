# DHT routing

GeoMesh relies on the geographic position of the recipient of the packet.

However, nodes should be identifiable by their unique address only, and nodes might move around over time.

Therefore, we need a way to look up the location of a node while only given its' unique 128-bit ID.

There are several approaches for this, but the only scalable approach to this that I know of is to use a distributed
hash table (DHT hereafter).

If Alice wants to send a message to Bob, but doesn't know Bob's location, Alice will first use DHT routing to get
a packet to Bob containing a request for Bob to tell his location to Alice, as well as Alice's location.

When Bob receives this package, he sends a response containing his location back to Alice. Since Alice included her
location in the request, Bob can simply use traditional Geographic routing to get the message to where it needs to go.

## How does Kademlia-style routing work?

In a DHT, each unique identifier is interpreted as a number, and this number is then interpreted as a position in
1-dimensional space. Messages are then greedily routed towards a specific point in that identifier space.

In Kademlia-syle routing, the distance between two nodes is calculating by taking the bitwise XOR of their unique
identifiers, and interpreting the resulting string of bits as an unsigned integer.

Each node has a routing table. To keep this routing table small, each node only remembers a logarithmic number
of other nodes.

It will retain the nodes that are a power of two distance away from it or, if a node with a perfect 2^n distance cannot
be found, it will remember the node that is closest to that distance **from above**.

For example, if the ID of a node ranged between 0000 and 1111, node with ID 0000 (0) would have routing table entries for
nodes 0001 (2^0), 0010 (2^1), 0100 (2^2) and finally 1000 (2^3).

If, for example, node 1000 didn't exist, node 0000 would store a reference to 1001 if it existed instead. (**Not**
 node 0111.)

## What kind of messages are involved?

See [Formal Packet Definitions](formal_packet_definition.md) for exactly how each packet is structured.

The information in this section is loosely based on [http://tutorials.jenkov.com/p2p/messages.html].



