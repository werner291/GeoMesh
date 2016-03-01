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
