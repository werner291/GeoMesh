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

#ifndef GEOMESH_LOCALPACKETHANDLER_H
#define GEOMESH_LOCALPACKETHANDLER_H

#include <map>
#include "VirtualLocationManager.hpp"
#include "Logger.hpp"
#include "UniqueAddress.hpp"
#include "Packet.hpp"

class Router;
class LocationLookupManager;

/**
 * Class responsible for handling sending and receiving packets from and to the local system.
 */
class LocalPacketHandler {

    typedef std::function<void (int messageType,
                                Address from,
                                GPSLocation fromLocation,
                                uint8_t* message,
                                size_t messageSize)> LocalPacketListener;

    typedef std::function<void (int messageType,
                                Address destination,
                                GPSLocation destinationLocation,
                                const uint8_t* payload,
                                size_t payloadSize)> sendPacketWithLocationFunction;

    std::map<int, std::vector<LocalPacketListener> > localDeliveryListeners;

    VirtualLocationManager& locationManager;
    Address localAddress;

    std::function<bool (PacketPtr)> sendPacket;

public:

    /**
     * Create a new LocalPacketHandler.
     */
    LocalPacketHandler(VirtualLocationManager& locationManager,
                       Address localAddress,
                       std::function<bool (PacketPtr)> sendPacket) :
            locationManager(locationManager),
            localAddress(localAddress),
            sendPacket(sendPacket) {}

    void addLocalPacketListener(LocalPacketListener listener, int messageType) {
        localDeliveryListeners[messageType].push_back(listener);
    }

    /**
     * Called from the Router when a packet with this node's address as the destination is received.
     */
    void handleLocalPacket(PacketPtr packet);

    /**
     * Send a datagram from this node with a specified payload.
     * Delivery is NOT guaranteed, and NO GUARANTEES are made about delays.
     * Will return immediately.
     *
     * @param messageType The message type to put in the header.
     * @param destination The address of the receiving node.
     * @param destinationLocation A more or less accurate Location of the destination node.
     * @param payload Pointer to a byte array representing the payload.
     * @param payloadSize The length/size of the payload in bytes. (<= MAX_PAYLOAD_SIZE)
     */
    void sendFromLocal(int messageType,
                       Address destination,
                       GPSLocation destinationLocation,
                       const uint8_t* payload,
                       size_t payloadSize);

public:
    void setSendPacketStrategy(const std::function<bool(PacketPtr)> &sendPacket) {
        LocalPacketHandler::sendPacket = sendPacket;
    }

};


#endif //GEOMESH_LOCALPACKETHANDLER_H
