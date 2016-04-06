//
// Created by Werner Kroneman on 28-01-16.
//

#include "UDPInterface.hpp"

#include "UDPManager.hpp"

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include "../Logger.hpp"


UDPInterface::UDPInterface(UDPManager* udpMan) {
    UDPInterface::udpMan = udpMan;
    nextPacketNumber = 0;
}

bool UDPInterface::sendData(PacketPtr data) {
    // The UDPInterface cannot send the data, only the UDPManager can do that
    return udpMan->sendMessage(data, this);
}

void UDPInterface::fragmentReceived(UDPFragmentPtr frag) {
    // Hash the packet number to choose a reception buffer.
    // This way, they are used sequentially as the packet number increases
    UDPReceptionBuffer& reception = receptionBuffers[frag->getPacketNumber() % UDP_RECEPTION_BUFFER_COUNT];

    reception.receive(frag);

    // If this fragment completed the packet, send it further into the system.
    if (reception.isFullPacketAvailable()) {
        Logger::log(DEBUG, "Reconstructed packet " + std::to_string(frag->getPacketNumber()));
        packetReceived(reception.getReconstructedPacket());
    }
}