//
// Created by Werner Kroneman on 28-01-16.
//

#include "UDPInterface.h"

#include "UDPManager.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include "../Logger.h"


UDPInterface::UDPInterface(UDPManager* udpMan) {
    UDPInterface::udpMan = udpMan;
    nextPacketNumber = 0;
}

bool UDPInterface::sendData(PacketPtr data) {
    
    return udpMan->sendMessage(data, this);


}

void UDPInterface::fragmentReceived(UDPFragmentPtr frag) {
    UDPReceptionBuffer& reception = receptionBuffers[frag->getPacketNumber() % UDP_RECEPTION_BUFFER_COUNT];

    reception.receive(frag);

    if (reception.isFullPacketAvailable()) {
        Logger::log(DEBUG, "Reconstructed packet " + std::to_string(frag->getPacketNumber()));
        packetReceived(reception.getReconstructedPacket());
    }
}