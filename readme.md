
# GeoMesh

## What is it?

Geomesh is a Layer 3 mesh network protocol, intended for efficient routing of packets through a network without a central
authority, while allowing any node in the network to choose their own address.

## How it is different from other meshnet protocols?

GeoMesh intents to address the scalability problems that other meshnets have. Traditionally, packets are routed
using XOR distance between node addresses, where a packet woud be greedily routed from one node to another based
on whichever neighbour is closest in terms of XOR distance.

This works, and eventually gets the message where it needs to go in O(log(n)) hops through the network (where n is the
number of nodes).

However, there is no useful relation between the position of a node in XOR space, and the position of that node in the
real world. A message that takes a short route through XOR space might go all the way through Timbuktu before reaching
the house across the street. Some networks use tricks to cut down on this behaviour, but the fundamental problem remains.

GeoMesh aims to solve this by using the coordinates of the location of each node (or an approximation thereof) to allow
nodes to send packets in the right direction without forcing them to know exactly where every single recipient node is.

## What are the main ideals of the project?

To create a network that focuses on:

1. Efficent routing: get a message where it needs to go while avoiding excessive detours

1. Goal-centered: The primary goal of this system is to route packets from A to B. If it does something else at some point,
                 this must be so as to improve routing.

1. Scalability: avoid causing the router explode once the network gets big enough.

1. Openness: Anyone is allowed on the network. Find a nearby peer, and you're in.

1. Freedom: All nodes should be free to do exchange information, as long as they do not prevent other nodes from doing the same.

1. Open source: The more eyes on the codebase, the more mistakes are spotted and the more ideas are applied.

1. Re-use of as much as possible of what is already there: the routing software should work on existing routing hardware, if possible even on low-end hardware with limited computing power and memory. The protocol avoids touching other layers than layer 3 of the OSI model as much as possible. Requiring the installation of special software on the nodes is permitted, but it should be possible to bridge the network over other existing networks.

## What about anonymity?

Every packet in the system will carry more or less accurate location coordinates of the recipient, which is bad for anonymity.

However, anonymity is not a goal for GeoMesh, as this would distract from the main purpose of routing packets.

If you do want anonymity, use a tool meant for the purpose like a proxy or some anonymity network like The Onion Router (TOR).
There is nothing in the protocol that prevents a system like TOR from running (even partially) on top of GeoMesh, but nothing
forces it onto those who don't want it.

## Is it secure?

Once again, getting information where it needs to go is the primary goal, and security on the network will only focus on
said primary goal. For example, the network should be resistant against malfunctioning or malicious nodes that prevent
correct packet routing or cause excessive unneccesary load.

Use tools like SSL or SSH if you want to send any kind of sensitive information. This is encouraged, even, but not
appropriate as a fundamental part of GeoMesh.