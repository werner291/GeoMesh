#Reference implementation architecture

The GeoMesh reference implementation is designed to be clean and modular.

## Packet flows

This is a flow chart of how information flows through the different modules.

<img src="packet_flows.svg">

### Outbound packets

First, an IPv6 packet is sent into the tunnel interface by the host system. This tunnel interface delivers
the packet to the LocalInterface.

The LocalInterface reads the destination address from the IPv6 header, and asks the LocationLookupManager (LLM)
whether the location of the node with that address is known. If it is known, the LocalInterface sends the packet
to the LocalHandler with the destionation address and location, specifying the original packet as the payload.
If it is not known, the LocalInterface puts the IPv6 packet into a queue, and requests a location lookup from
the LLM. After a while, the LLM will have completed the lookup, and notifies the LocalInterface, which will pull
the packet out of the queue and send it to the LocalHandler in the same way as before.

The LocalHandler creates a GeoMesh packet with the specified destination information, writes the IPv6 packet as payload,
and completes the source header fields with the address and location of the sending node. It then sends the packet to
the Router to handle it.

The Router will then decide which output interface to send the message to (see: TODO: write that part), and passes
the packet, along with an interface ID, to the LinkManager. The LinkManager then passes the packet to the Interface
with that ID. The Interface object will send the packet somewhere else, depending on the type of the interface.