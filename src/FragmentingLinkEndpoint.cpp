/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Logger.hpp"
#include "FragmentingLinkEndpoint.hpp"

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>

FragmentingLinkEndpoint::FragmentingLinkEndpoint(FragmentHandlerCallback handler) 
    : fragmentHandler(handler) {
    nextPacketNumber = 0;
}

bool FragmentingLinkEndpoint::sendData(PacketPtr packet) {
    
    uint16_t packetNum = nextPacketNumber++;
    
    int maxFragSize = 500;

    for (int fragStart = 0; fragStart < packet->getDataLength(); fragStart += maxFragSize) {

        int fragLength = std::min(maxFragSize, packet->getDataLength() - fragStart);

        PacketFragmentPtr frag(new PacketFragment(packet->getData()+fragStart,
                    fragLength, false));

        frag->setPacketNumber(packetNum);
        frag->setPacketLength(packet->getDataLength());
        frag->setDestinationInterfaceID(mRemoteIface);
        frag->setFragmentStart(fragStart);
        frag->setFragmentLength(fragLength);

        fragmentHandler(frag, iFaceID);
    }
    return true;

}

void FragmentingLinkEndpoint::fragmentReceived(PacketFragmentPtr frag) {
    // Hash the packet number to choose a reception buffer.
    // This way, they are used sequentially as the packet number increases
    PacketDefragmenter& reception = receptionBuffers[frag->getPacketNumber() % UDP_RECEPTION_BUFFER_COUNT];

    reception.receive(frag);

    // If this fragment completed the packet, send it further into the system.
    if (reception.isFullPacketAvailable()) {
        Logger::log(DEBUG, "Reconstructed packet " + std::to_string(frag->getPacketNumber()));
        packetReceived(reception.getReconstructedPacket());
    }
}
