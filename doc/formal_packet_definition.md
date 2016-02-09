#GeoMesh packet header definitions

There are two types of packets in GeoMesh:

* Payload packages, carrying data for layer 4 protocols
* Location advertisement packages, advertising the location and address of a node.

##Payload package header

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
<tr>
<td>4</td>
<td colspan="4">Destination latitude</td>
</tr>
<tr>
<td>8</td>
<td colspan="4">Destination longitude</td>
</tr>
<tr>
<td>12</td>
<td colspan="4">Destination altitude</td>
</tr>
<tr>
<td>16</td>
<td colspan="4">Face routing start distance</td>
</tr>
<tr>
<td>20</td>
<td colspan="4">Face routing search range</td>
</tr>
<tr>
<td>24</td>
<td colspan="4">Default IPv6 header</td>
</tr>
<tr>
<td>...</td>
<td colspan="4"></td>
</tr>
<tr>
<td>64</td>
<td colspan="4">Payload start</td>
</tr>
</table>

* Destination latitude / longitude: Signed 32-bit integer representing an angle. Convert to degrees by multiplying by
180 and dividing by 2^31. Converting the number to a double beforehand is recommended.

* Destination altitude. 32-bit IEEE floating point number, representing altitude in meters. 0 is at ground level.
Accuracy of this value is mostly important when taken relative to other nearby nodes, especially if nodes are above one
another (in a tall building for example), but less relevant as nodes are further apart.

* Face routing start distance: 32-bit IEEE floating point number representing the distance from the destination location
at the start of face routing in meters.

* Default IPv6 header: The GeoMesh header is an extension of the normal IPv6 header, and the extra information can simply be
prepended to the IPv6 packet, instead of wrapping the whole packet. The original IPv6 packet can be retrieved by removing
the first 24 bytes of the GeoMesh packet.

## DHT routing table copy request

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
<tr>
<td>4</td>
<td colspan="4" rowspan="4">Route label</td>
</tr>
</table>

## DHT routing table copy request response

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
<tr><td>4</td><td colspan="4" rowspan="4">Route label</td></tr>
<tr><td>20</td><td colspan="4">Number of entries</td></tr>
<tr><td>24</td><td colspan="4" rowspan="4">Node 1 ID</td></tr>
<tr><td>28</td><td colspan="4" rowspan="4">Node 1 Route label</td></tr>
<tr><td>...</td><td colspan="4" rowspan="8">...</td></tr>
<tr><td>24 + 32 * n</td><td colspan="4" rowspan="4">Node *n* ID</td></tr>
<tr><td>28 + 32 * n</td><td colspan="4" rowspan="4">Node *1*n Route label</td></tr>
</table>

## Location Lookup Packet

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
<tr>
<td>4</td>
<td colspan="4" rowspan="4">Destination address</td>
</tr>
<tr>
<td>20</td>
<td colspan="4" rowspan="4">Source address</td>
</tr>
<tr>
<td>24</td>
<td colspan="4">Sender latitude</td>
</tr>
<tr>
<td>28</td>
<td colspan="4">Sender longitude</td>
</tr>
<tr>
<td>32</td>
<td colspan="4">Sender altitude</td>
</tr>
</table>

## Location Lookup Response

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
<tr>
<td>4</td>
<td colspan="4" rowspan="4">Source address</td>
</tr>
<tr>
<td>24</td>
<td colspan="4">Source latitude</td>
</tr>
<tr>
<td>28</td>
<td colspan="4">Source longitude</td>
</tr>
<tr>
<td>32</td>
<td colspan="4">Source altitude</td>
</tr>
<tr>
<td>20</td>
<td colspan="4" rowspan="4">Destination address</td>
</tr>
<tr><td>28</td></tr>
<tr><td>32</td></tr>
<tr><td>36</td></tr>
<tr><td>40</td><td colspan="4">Destination latitude</td></tr>
<tr><td>44</td><td colspan="4">Destination longitude</td></tr>
<tr><td>48</td><td colspan="4">Destination altitude</td></tr>
<tr><td>52</td><td colspan="4" rowspan="2">Timestamp</td></tr>
<tr><td>56</td></tr>
<tr><td>60</td><td colspan="4" rowspan="2">Valid For</td></tr>
<tr><td>66</td></tr>
</table>

* Source address: The unique address of the node sending the location lookup response (not the one that made the request)

* Source latitude / longitude / altitude: The geographical position of the node sending the lookup response

* Destination address: The unique address of the node receiving the location lookup response (the one that wants to know the location)

* Destination latitude / longitude / altitude: The geographical position of the node receiving the lookup response

* Timestamp: The 64-bit Unix timestamp at which the location of the sending node was valid.

* Valid For: The 64-bit Unix timestamp after which the receiving node should consider
  the location information to be ureliable. Please note when implementing this, that the virtual location of a node
  can change even when it is stationary in the real world.

Two nodes desiring to communicate SHOULD re-send location lookup response packets before the inforation expires.