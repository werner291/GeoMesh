//
// Created by Werner Kroneman on 28-01-16.
//

#include "UDPInterface.h"

#include <sys/socket.h>
#include <fcntl.h>
#include <errno.h>
#include "../Logger.h"

UDPInterface::UDPInterface() {

    socketID = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in localAddr;

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = 0;

    bind(socketID, (struct sockaddr *) &localAddr, sizeof(localAddr));

    /* Now bound, get the address */
    socklen_t slen = sizeof(localAddr);
    getsockname(socketID, (struct sockaddr *) &localAddr, &slen);

    mLocalUDPport = ntohs(localAddr.sin_port);
    // Enable non-blocking IO.
    int flags = fcntl(socketID, F_GETFL, 0);
    fcntl(socketID, F_SETFL, flags | O_NONBLOCK);

}

void UDPInterface::pollMessages() {

    int nbytes = recvfrom(socketID, mReceptionBuffer, MAX_PACKET_SIZE, 0, NULL, NULL);

    if (nbytes > 0) {

        PacketPtr packet = Packet::createFromData(mReceptionBuffer, MAX_PACKET_SIZE);

        dataArrivedCallback(packet, iFaceID);

    } else if (nbytes == 0) {
        // do nothing
    } else {
        int err = errno;

        if (!(err == EWOULDBLOCK || err == EAGAIN)) {
            Logger::log(LogLevel::ERROR,
                        "UDPInterface: receive error: " + std::string(strerror(err)));
        }
        // Elseo there was nothing to be read, do nothing
    }

}

bool UDPInterface::sendData(PacketPtr data) {
    
    assert(peerAddress.sin_family == AF_INET);

    int result = sendto(socketID,
                        data->getData(),
                        data->getDataLength(),
                        0,
                        (struct sockaddr *) &peerAddress,
                        sizeof(peerAddress));

    if (result < 0) {
        Logger::log(LogLevel::ERROR, "UDPInterface: error while sending: " + std::string(strerror(errno)));
    }

    return result == 0;
}
