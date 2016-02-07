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

    sockaddr_in sin;

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = 0;

    bind(socketID, (struct sockaddr *) &sin, sizeof(sin));

    /* Now bound, get the address */
    socklen_t slen = sizeof(sin);
    getsockname(socketID, (struct sockaddr *) &sin, &slen);

    mLocalUDPport = ntohs(sin.sin_port);
    // Enable non-blocking IO.
    int flags = fcntl(socketID, F_GETFL, 0);
    fcntl(socketID, F_SETFL, flags | O_NONBLOCK);

}

void UDPInterface::pollMessages() {

    int nbytes = recvfrom(socketID, mReceptionBuffer, MAX_PACKET_SIZE - IPv6_START, 0, NULL, NULL);

    if (nbytes > 0) {

        DataBufferPtr packet(new std::vector<char>(nbytes));

        memcpy(packet->data(), mReceptionBuffer, nbytes);

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

bool UDPInterface::sendData(DataBufferPtr data) {

    int result = sendto(socketID,
                        data->data(),
                        data->size(),
                        0,
                        (struct sockaddr *) &peerAddress,
                        sizeof(peerAddress));

    if (result < 0) {
        Logger::log(LogLevel::ERROR, "UDPInterface: error while sending: " + std::string(strerror(errno)));
    }

    return result == 0;
}