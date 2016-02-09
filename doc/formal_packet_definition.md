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

* Destination altitude. 32-bit IEEE floating point number, representing altitude in meters. 0 is at ground level on
 a relatively flat surface. In case of a mountainous landscape, use the bottom of the mountain. Accuracy of this value is mostly
 important when taken relative to direct neighbours, especially if nodes are above one another (in a tall building for example),
 but less relevant as nodes are more far apart.

* Face routing start distance: 32-bit IEEE floating point number representing the distance from the destination location
   at the start of face routing in meters.

* Default IPv6 header: The GeoMesh header is an extension of the normal IPv6 header, and the extra information can simply be
 prepended to the IPv6 packet, instead of wrapping the whole packet. The original IPv6 packet can be retrieved by removing
 the first 24 bytes of the GeoMesh packet.