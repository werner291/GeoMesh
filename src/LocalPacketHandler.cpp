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

#include "LocalPacketHandler.hpp"

void LocalPacketHandler::handleLocalPacket(PacketPtr packet) {
    auto itr = localDeliveryListeners.find(packet->getMessageType());

    if (itr != localDeliveryListeners.end() && !itr->second.empty()) {
        for (LocalPacketListener& listener : itr->second) {
            listener(packet->getMessageType(),
                     packet->getSourceAddress(),
                     packet->getSourceLocation(),
                     packet->getPayload(),
                     packet->getPayloadLength());
        }
    } else {
        Logger::log(LogLevel::WARN, "Message received with unknown type "
                                    + std::to_string(packet->getMessageType()));
    }
}

void LocalPacketHandler::sendFromLocal(int messageType, Address destination, GPSLocation destinationLocation,
                                       const uint8_t *payload, size_t payloadSize) {

    assert(payloadSize <= MAX_PAYLOAD_SIZE);

    PacketPtr packet = std::make_shared<Packet>(localAddress,
                                                locationManager.getLocation(),
                                                destination,
                                                destinationLocation,
                                                messageType,
                                                payloadSize);

    memcpy(packet->getPayload(), payload, payloadSize);

    sendPacket(packet);

}
