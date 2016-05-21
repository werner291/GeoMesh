//
// Created by System Administrator on 2/6/16.
//

#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <string.h>
#include <arpa/inet.h>
#include "UnixSocketsFunctions.hpp"
#include "../Logger.hpp"

using namespace std;

int receiveMessage(int sock, uint8_t *buffer, size_t buffSize) {

    int nbytes = recvfrom(sock, buffer, buffSize, O_NONBLOCK, NULL, NULL);

    if (nbytes > 0) {

        int aftype = ntohs(((uint16_t *) buffer)[1]);

        if (aftype == AF_INET6) {
            return nbytes;
        } else {
            Logger::log(LogLevel::ERROR,
                        "receiveMessage: invalid aftype: " + std::to_string(aftype));
        }


    } else if (nbytes == 0) {
        // do nothing
    } else {
        int err = errno;

        throw runtime_error("ReceiveMessage: receive error: " + std::string
                                                                     (strerror(err)));
        // Else there was nothing to be read, do nothing
    }

    return -1;
}
