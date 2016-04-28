# Location lookup algorithm: proof of post-convergence lookup guarantee

In this document, I will prove that, once the location lookup contacts table has converged (it stops changing),
location lookups are guaranteed to succeed given that messages arrive properly if sent.

Let's define the following:

* N - All nodes in the network. Every element *n* of *N* is a node, and has a unique ID. The notion of addresses
      and nodes will be used somewhat interchangably throughout this document, as the relationship between nodes
      and ID's is 1-to-1. We will assume that there are always at least 2 nodes.

* knows - The "knows" relation. If, for arbitrary nodes a and b, "a knows b" is true, a can send messages to b.
  It is basically the node's contacts table (see DHT.md), but then for all nodes at the same time to make reasoning
  easier. We know that *knows* is reflexive (every node can reach itself) and symmetric (a knows b iff b knows a).

* canReach - If, for arbitrary node a and b, "a canReach b" is true, a can send messages to b with guaranteed delivery.

* Find closest request: a tuple (q,r), consisting of a *q*, the query address, and r, the requester address.

Also, 

## Refresh rule

The refresh rule is the rule by which all nodes try to update their contacts table (their relatives in *knows*)
in order to guarantee delivery of a message. In this proof, we will set the rule as follows:

  At every time step, every node sends "find closest" query for all 1-bit variations of its' own address. It stores
  the result of those queries in its contacts. The node that sends the response also puts the requester in its'
  contacts. (Which makes that *knows* is symmetric.)

## Relay rule

The relay rule is the rule by which a node either relays or respons to a "find closest" request.

Let *a* be the node receiving a request (q,r).

  If there is a node *b* such that *b.address ^ q < a.address ^ q*, then the node must relay the request
  to *b*. If there are multiple such *b*, it will relay it to the *b* that minimises *b.address ^ q*.

  If there is no such *b*, *a* will respond to the request by itself. This effectively means that
  *r knows_t+1 a* and vice-versa.

Notation:

* t : The time step, we start at t = 0
* A_t: For some arbitrary time-dependant object *A*, *A_t* denotes the state of *A* at time *t*.
       *knows* and *canReach* are both time-dependant. 
* t_c: The convergence time. It is the minimal *t* for which *knows* non longer changes.

Our claim:

  If *knows* is a connected graph at t = 0, then canReach is a connected graph at t = t_c.

## Proof

Let *a,b* be arbitrary different nodes.

###   


  Suppose, for example, that node 00 wants to send a message to node 10. The destination address of the message is
  thus  There is also another node 01. We only know
  that "00 knows 01" and "01 knowjs 10". 
