//
// Created by System Administrator on 3/18/16.
//

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

void LocalPacketHandler::sendFromLocal(int messageType, Address destination, Location destinationLocation,
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
