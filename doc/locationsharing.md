# Location advertisement

## Location packets

A location packet is a special packet containing the following information

* A location, consisting of an X and a Y value
* A hop counter, indicating how far this packet has already travelled.

## Routing table

* In this document, we will only concern ourselves with the greedy routing table, later referred to simply as the
"routing table".

The routing table is abstractly defined as follows: A set of routing rules, all of which have a unique location.

A routing rule, in turn, is defined as follows: a structure containing:
 * A location consisting of a X and a Y coordinate.
 * An interface ID that corresponds to one of the interfaces of the router that owns the routing table.

If a new routing rule is inserted into the table that has an equal location to a previous rule, replace that
previous rule by the rule to be inserted.

A location *A* is equal to another *B* if and only if the coordinates of *A* and *B* are equal.

Proposal: *A* and *B* would also be equal if they were within a certain threshold distance *d<sub>max</sub>*
from eachother, where *d<sub>max</sub>* increases as *A* and *B* are further away from the routing table's owner's position,
and where *d<sub>max</sub>* is also multiplied by a factor indicating "pruning aggressiveness", which in turn is a function
of the memory usage of the table.

## Start of location packet

When a router A establishes a link with another router B, A sends a packet to B containing A's location.

By following the same rules, B also sends its' location to A.

All routers also periodically send their location to all linked neighbour nodes.

## Storage of location information and retransmission rules

When a node *A* receives an location advertisement packet L, it also notes the ID of the interface the packet was
received from.

Next, *A* determines whether the information in L is useful (see below).

## Criterion of usefulness

If yes, *A* stores the information in *L*, and repeats *L* to all its neighbours of *A* except the neigbour that *L*
originates from. (When in doubt about which neighbour *L* came from, send it to suspected origins as well.)

A location packet *L* is useful to *A* if and only if *L* contains information that, were it to be inserted in
*A*'s routing table, enables *A* to route packets to locations it could not have reached before, or to route
packets significantly more efficently.
 
This is obviously hard to determine, but a solution is proposed below and is to be implemented in the reference implementation.
Any implementation should take care so as to avoid unneccesary replication of a location message.

1. If the routing table is empty, return that *L* is useful.

1. If there is an entry in the routing table with a similar location to *L*:

    1. If the cost advertised by *L* is lower, *L* is useful

    2. Else, *L* is not useful

1. If there is no similar location to *L* in the routing table, *L* is useful.
