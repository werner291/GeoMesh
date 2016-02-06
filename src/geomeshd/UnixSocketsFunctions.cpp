//
// Created by System Administrator on 2/6/16.
//

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include "UnixSocketsFunctions.h"
#include "../constants.h"
#include "../Logger.h"

int receiveMessage(int sock, char *buffer, size_t buffSize) {

    int nbytes = recvfrom(sock, buffer, MAX_PACKET_SIZE - IPv6_START - 4, O_NONBLOCK, NULL, NULL);

    if (nbytes > 0) {

        int aftype = ntohs(((uint16_t *) buffer)[1]);

        if (aftype == AF_INET6) {
            return nbytes;
        } else {
            Logger::log(LogLevel::ERROR,
                        "TunnelDeliveryInterface_Apple: invalid aftype: " + std::to_string(aftype));
        }


    } else if (nbytes == 0) {
        // do nothing
    } else {
        int err = errno;

        if (!(err == EWOULDBLOCK || err == EAGAIN)) {
            Logger::log(LogLevel::ERROR,
                        "TunnelDeliveryInterface_Apple: receive error: " + std::string(strerror(err)));
        }
        // Else there was nothing to be read, do nothing
    }

    return -1;
}