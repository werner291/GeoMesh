//
// Created by System Administrator on 3/18/16.
//

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
                                Location fromLocation,
                                uint8_t* message,
                                size_t messageSize)> LocalPacketListener;

    typedef std::function<void (int messageType,
                                Address destination,
                                Location destinationLocation,
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
                       Location destinationLocation,
                       const uint8_t* payload,
                       size_t payloadSize);

public:
    void setSendPacketStrategy(const std::function<bool(PacketPtr)> &sendPacket) {
        LocalPacketHandler::sendPacket = sendPacket;
    }

};


#endif //GEOMESH_LOCALPACKETHANDLER_H
