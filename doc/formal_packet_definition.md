#GeoMesh packet header definitions

There are two types of packets in GeoMesh:

* Payload packages, carrying data for layer 4 protocols
* Location advertisement packages, advertising the location and address of a node.

##Geomesh Packet Header

All GeoMesh packets MUST begin with a header as specified below.

<table>
<tr>
<th>Octet</th>
<th>0</th>
<th>1</th>
<th>2</th>
<th>3</th>
</tr>
<tr>
<td>0</td>
<td colspan="2">Protocol Version</td>
<td>Message type</td>
<td>Routing Mode</td>
</tr>
<tr><td>4</td><td colspan="4" rowspan="4">Source address</td></tr>
<tr><td>8</td></tr>
<tr><td>12</td></tr>
<tr><td>16</td></tr>
<tr><td>20</td><td colspan="4">Source latitude</td></tr>
<tr><td>24</td><td colspan="4">Source longitude</td></tr>
<tr><td>28</td><td colspan="4">Source altitude</td></tr>
<tr><td>32</td><td colspan="4" rowspan="4">Destination address</td></tr>
<tr><td>36</td></tr>
<tr><td>40</td></tr>
<tr><td>44</td></tr>
<tr><td>48</td><td colspan="4">Destination latitude</td></tr>
<tr><td>52</td><td colspan="4">Destination longitude</td></tr>
<tr><td>56</td><td colspan="4">Destination altitude</td></tr>
<tr><td>60</td><td colspan="4">Face routing start distance</td></tr>
<tr><td>64</td><td colspan="4">Face routing search range</td></tr>
<tr><td>...</td><td colspan="4">Payload</td></tr>
</table>

* Destination latitude / longitude: Signed 32-bit integers representing an angle. Convert to degrees by multiplying by
180 and dividing by 2^31. Converting the number to a double beforehand is recommended.

* Destination altitude. 32-bit IEEE floating point number, representing altitude in meters. 0 is at ground level.
Accuracy of this value is mostly important when taken relative to other nearby nodes, especially if nodes are above one
another (in a tall building for example), but less relevant as nodes are further apart.

* Face routing start distance: 32-bit IEEE floating point number representing the distance from the destination location
at the start of face routing in meters.

* Default IPv6 header: The GeoMesh header is an extension of the normal IPv6 header, and the extra information can simply be
prepended to the IPv6 packet, instead of wrapping the whole packet. The original IPv6 packet can be retrieved by removing
the first 24 bytes of the GeoMesh packet.

* Payload: An arbitrary-length payload, must correspond to the message type.

## DHT routing table copy request

A DHT routing table copy request packet has an empty body, since the header has sufficient information to fulfill
the request.

Note: In general, a routing table copy request SHOULD NOT be relayed, as it generally travels only 1 hop.
This is to prevent spoofing.

## DHT routing table copy request response

A DHT routing table copy response contains a list of routing table entries.
Note: In general, a routing table copy request response SHOULD NOT be relayed, as it generally travels only 1 hop.

Also, if a node receives such a routing table copy without requesting it, the node SHOULD ignore the packet.

<table>
<tr><th>Octet</th><th>0</th><th>1</th><th>2</th><th>3</th></tr>
<tr><td>0</td><td colspan="4">Number of entries</td></tr>
<tr><td>4</td><td colspan="4" rowspan="4">Peer 1 address</td></tr>
<tr><td>8</td></tr>
<tr><td>12</td></tr>
<tr><td>16</td></tr>
<tr><td>20</td><td colspan="4">Peer 1 latitude</td></tr>
<tr><td>24</td><td colspan="4">Peer 1 longitude</td></tr>
<tr><td>28</td><td colspan="4">Peer 1 altitude</td></tr>
<tr><td>32</td><td colspan="4" rowspan="2">Peer 1 entry expires</td></tr>
<tr><td>36</td></tr>
<tr><td>...</td><td colspan="4">...</td></tr>
<tr><td>4 + n * 36</td><td colspan="4" rowspan="4">Peer 1 address</td></tr>
<tr><td>8 + n * 36</td></tr>
<tr><td>12 + n * 36</td></tr>
<tr><td>16 + n * 36</td></tr>
<tr><td>20 + n * 36</td><td colspan="4">Peer *n* latitude</td></tr>
<tr><td>24 + n * 36</td><td colspan="4">Peer *n* longitude</td></tr>
<tr><td>28 + n * 36</td><td colspan="4">Peer *n* altitude</td></tr>
<tr><td>32 + n * 36</td><td colspan="4" rowspan="2">Peer *n* entry expires</td></tr>
<tr><td>36 + n * 36</td></tr>
</table>

* Number of entries: 32-bit unsigned integer indicating the number of entries.

* Peer *i* address (for some integer *i*): The unique identifier of peer *i*.

## Location Lookup Packet

This packet also has an empty body.

<table>
<tr><th>Octet</th><th>0</th><th>1</th><th>2</th><th>3</th></tr>
<tr><td>4</td><td colspan="4" rowspan="4">Requester address</td></tr>
<tr><td>8</td></tr>
<tr><td>12</td></tr>
<tr><td>16</td></tr>
<tr><td>20</td><td colspan="4">Requester latitude</td></tr>
<tr><td>24</td><td colspan="4">Requester longitude</td></tr>
<tr><td>28</td><td colspan="4">Requester altitude</td></tr>
<tr><td>4</td><td colspan="4" rowspan="4">Target address</td></tr>
<tr><td>8</td></tr>
<tr><td>12</td></tr>
<tr><td>16</td></tr>
</table>

* Requester address: The unique address of the node requesting the location

* Requester lat/lon/alt: The location of the node requesting the location

* Target address: The address of the node whose location the requester would like to know.

For relay rules, see [DHT.md].

## Location Lookup Response

<table>
<tr><th>Octet</th><th>0</th><th>1</th><th>2</th><th>3</th></tr>
<tr><td>0</td><td colspan="4" rowspan="2">Timestamp</td></tr>
<tr><td>4</td></tr>
<tr><td>8</td><td colspan="4" rowspan="2">Valid For</td></tr>
<tr><td>12</td></tr>
</table>

The header already contains both the source and destination location, so that should be sufficient as a
response.

Two nodes desiring to communicate SHOULD re-send location lookup response packets before the information expires.

Proposal/request for comments: Should this message include a cryptographic signature?
Potentially malicious nodes could respond to a location lookup with a fake response, and since such a request
tends to travel quite far, some kind of authentication mechanism would be nice.

Another proposal would be for a node to send out multiple request in multiple directions, and use the response that is
produced most often.