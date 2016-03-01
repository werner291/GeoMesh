# Bridging GeoMesh over Internet or other IP networks

If two GeoMesh nodes are connected to the Internet, some other network that uses IP,
or a network that can handle IP-like packets like CJDNS, a bridge can be created between
the two nodes.

GeoMesh uses a small, lightweigt protocol to send GeoMesh packets using UDP.

## UDP bridge protocol specification

Please note that this is **NOT** the GeoMesh protocol, and these packets **MUST NOT**
be treated as valid GeoMesh packets.

Since the minimum MTU of both IPv4 and Ipv6 is less than GeoMesh addresses

<table>
<tr>
<th>Octet</th>
<th>0</th>
<th>1</th>
<th>2</th>
<th>3</th>
</tr>
<tr><td>0</td><td colspan="2">Protocol Version</td><td colspan="2">Destination interface ID</td></tr>
<tr><td>4</td><td colspan="2">Packet number</td><td colspan="2">Packet length</td></tr>
<tr><td>8</td><td colspan="2">Fragment start</td><td colspan="2">Fragment length</td></tr>
<tr><td>12</td><td colspan="4">Payload...</td></tr>
</table>

* Protocol version: 16-bit unsigned int in network byte order representing the UDP bridge protocol version.
    (NOT the GeoMesh protocol version, which is irrelevant in this context.)

* Packet length: 16-bit unsigned int representing the length of the original packet. (Not the fragment.)

* Packet number: A 16-bit unsigned integer designating which packet this fragment belongs to.
                 To be incremented for each packet, wraps around back to 0 if the value is exceeded.

* Destination interface ID: The interface ID on the recipient node.

* Fragment start: 16-bit unsigned int The starting octet of where the fragment fits in the original packet

* Fragment length: 16-bit unsigned int The length of the fragment