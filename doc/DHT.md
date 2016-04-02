# Location lookup DHT

GeoMesh relies on the geographic position of the recipient of the packet.

However, nodes should be identifiable by their unique address only, and nodes might move around over time.

Therefore, we need a way to look up the location of a node while only given its' unique 128-bit ID.

There are several approaches for this, but the only scalable approach to this that I know of is to use a distributed
hash table (DHT hereafter). 

The location lookup system also works on a higher level than GeoMesh geographic routing: it does not at all take the actual
shape of the network into account, and therefore should be considered a somewhat seperate system. It can be bypassed completely
in case a node already knows the location of the destination by some other means.

## Routing table maintenance

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
 
## Comparison to Kademlia
 
The system is similar to Kademlia in that it uses the XOR-distance metric between addresses to route packets and the way
it chooses which entries to keep in the routing table, but will differs on certain major points:

The most important difference is that Kademlia uses an iterative approach where the original requester keeps sending FIND
requests based on the responses given to it by the nodes it queried previously while the location lookup service instead
uses a forwarding system, using a "route choice number" as a way of providing reduncancy. There is also no need to store
values in the system, only looking up nodes will be necessary.

## Messages and how to handle them

For a formal definition of the structure of these messages, see [formal_packet_definition.md].

### Find closest

This is a message containing the following fields:

* Requester ID / Address
* Requester location
* Query address
* Max hops
* Route choice: If non-zero, the message should not be routed straight towards the destination, but be forwarded to
                another contact whose address **is** closer than the current node is chosen pseudorandomly.
* A random query ID

When a node receives such a message, the node will look in its contacts table for contacts whose ID is closer to the query
address that the node's own addresses. This leaves us with two cases:

1. There are such contacts. If a contact's address exactly matches the query, the message is relayed to that contact.
   After that, if the *route choice* is 0, the message is relayed to the contact whose address is closest to the query.
   Finally, if the *route choice* is non-zero, one of the candidate contacts is chosen pseudorandomly, with a tendency
   to pick contacts closer to the query. On relay, the hop counter should be decremented.
   
2. No contact has an address closer to the query. The node sends a response to the original requester with its own
   contact information. The node might also want to store the requester's address and location in the contacts table.
   
If multiple contacts appear to be a good choice, the node MUST pick one, it MUST NOT duplicate the message to avoid
exponential duplication of the message.

Nodes other than the requester MUST NOT try to interpret the query ID or make assumptions about is, as it is dependant
on the implementation by the requester.

## Procedures and sending messages

### Joining the network

Alice joins a GeoMesh network by explicitely connecting to some direct neighbours, either by making some physical connection
or by connecting over some other network. The identifer and location of these neighbours can be found simply by sending
a message over the link to them. Alice adds the direct neighbours to the routing table. She has now joined the network,
but her routing table is rather small. She then starts a refresh procedure.

### Location lookup

The node is given an address, and the location corresponding to that address must be lookup up.

The node composes a number of "find closest" messages as follows:

* Requester ID / Address: The node's own address
* Requester location: The node's own location
* Query address: The address whose location must be looked up
* Max hops: The number of hops after which the network should drop the request, default 128
* Route choice: 0 for the first request, 1, 2, 3 etc... for further redundant requests
* A random query ID: Generate randomly (may be the same in redundant messages)

The default number of redundant messages in the reference implementation is 3.

The node then handles the message the same way as it would handle a "find closest" message it received from another node.

### Refresh procedure

The refresh procedure is a process that is carried out periodically by all nodes in the network, which aims at keeping
the routing table up to date and as close as possible to the optimal form where each entry is 2^n distance away.

1. The node generates a list of all 1-bit variations of its own address. These addresses will be exactly 2^n away from
   its own adress, and are thus the most desireable as entries in the routing table.
   
2. The node performs a location lookup for all these addresses.