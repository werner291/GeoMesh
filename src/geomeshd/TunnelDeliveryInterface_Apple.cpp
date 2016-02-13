//
// Created by System Administrator on 1/30/16.
//

#include <sys/sys_domain.h>
#include <sys/fcntl.h>
#include <sys/kern_control.h>
#include <sys/ioccom.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <regex>
#include <errno.h>
#include "TunnelDeliveryInterface_Apple.h"
#include "../Logger.h"
#include <iomanip>
#include "UnixSocketsFunctions.h"

#define UTUN_CONTROL_NAME "com.apple.net.utun_control"
#define UTUN_OPT_IFNAME 2

TunnelDeliveryInterface_Apple::TunnelDeliveryInterface_Apple(LocalInterface *localInterface,
                                                             const Address &iFaceAddress)
        : mLocalInterface(localInterface), iFaceAddress(iFaceAddress) {
    localInterface->setDataReceivedHandler(
            std::bind(&TunnelDeliveryInterface_Apple::deliverIPv6Packet, this, std::placeholders::_1));
}

void TunnelDeliveryInterface_Apple::startTunnelInterface() {

    mSocketId = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);

    if (mSocketId < 0) {
        Logger::log(LogLevel::ERROR, "socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL) " + std::string(strerror(errno)));
    }

    /* get/set the id */
    struct ctl_info info;
    memset(&info, 0, sizeof(info));
    strncpy(info.ctl_name, UTUN_CONTROL_NAME, strlen(UTUN_CONTROL_NAME));

    if (ioctl(mSocketId, CTLIOCGINFO, &info) < 0) {
        int err = errno;
        close(mSocketId);
        Logger::log(LogLevel::ERROR, "getting utun device id " + std::string(strerror(errno)));
    }

    addr.sc_id = info.ctl_id;

    addr.sc_len = sizeof(addr);
    addr.sc_family = AF_SYSTEM;
    addr.ss_sysaddr = AF_SYS_CONTROL;
    addr.sc_unit = 0; /* allocate dynamically */

    if (connect(mSocketId, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        int err = errno;
        close(mSocketId);
        Logger::log(LogLevel::ERROR, "connecting to utun device" + std::string(strerror(err)));
    }

    int maxNameSize = TUNInterface_IFNAMSIZ;

    /* retrieve the assigned utun interface name */
    if (getsockopt(mSocketId, SYSPROTO_CONTROL, UTUN_OPT_IFNAME,
                   iFaceName, (uint32_t *) &maxNameSize) >= 0) {

        Logger::log(LogLevel::INFO, "Initialized utun interface " + std::string(iFaceName));
    } else {
        int err = errno;
        close(mSocketId);
        Logger::log(LogLevel::ERROR, "getting utun interface name" + std::string(strerror(err)));
    }

    // Enable non-blocking IO.
    int flags = fcntl(mSocketId, F_GETFL, 0);
    fcntl(mSocketId, F_SETFL, flags | O_NONBLOCK);

    int n = MAX_PACKET_SIZE * 5;
    if (setsockopt(mSocketId, SOL_SOCKET, SO_SNDBUF, &n, sizeof(n)) == -1) {
        int err = errno;
        Logger::log(LogLevel::ERROR, "Error setting buffer size!" + std::string(strerror(err)));
        close(mSocketId);
    }

    assignIP();

    installRoute();

}



/**
 * Used a system() call to ifconfig. I was drowning in obscure compiler errors while trying to do it the proper ioctl() way.
 * If anyone reading this feels courageous... IMHO this works just fine
 */
void TunnelDeliveryInterface_Apple::assignIP() {

    std::regex utunReg("utun[0-9]+");

    // Prevent a nasty bash injection under the root user.
    if (!std::regex_match(std::string(iFaceName), utunReg)) {
        Logger::log(LogLevel::ERROR, "Invalid interface name " + std::string(iFaceName));
        return;
    };

    std::stringstream command;

    command << "ifconfig " << iFaceName << " inet6 add " << iFaceAddress.toString();

    Logger::log(LogLevel::INFO, "Assigned IPv6 address " + command.str());

    system(command.str().c_str());
}

void TunnelDeliveryInterface_Apple::installRoute() {

    std::regex utunReg("utun[0-9]+");

    // Prevent a nasty bash injection under the root user.
    if (!std::regex_match(std::string(iFaceName), utunReg)) {
        Logger::log(LogLevel::ERROR, "Invalid interface name " + std::string(iFaceName));
        return;
    };

    system(("route -q add -inet6 -net fcfd: -prefixlen 16 -interface " + std::string(iFaceName)).c_str());

    Logger::log(LogLevel::INFO,
                "Installed route. All traffic to fcfd:/16 will be sent through " + std::string(iFaceName) + ".");
};

void TunnelDeliveryInterface_Apple::deliverIPv6Packet(PacketPtr packet) {

    uint8_t buffer[MAX_PACKET_SIZE + 4];

    ((uint16_t *) buffer)[0] = htons(0);            // Always 0
    ((uint16_t *) buffer)[1] = htons(AF_INET6);   // Set to AF_INET6 so it is handled by the Internet stack.

    memcpy(buffer + 4, packet->getData(), packet->getDataLength());

    // Send to the local system.
    int result = send(mSocketId,
                      buffer,
                      packet->getDataLength() + 4,
                      0);
    //(struct sockaddr*) &addr,
    //sizeof(addr));

    if (result == -1) {
        int err = errno;
        Logger::log(LogLevel::ERROR,
                    "TunnelDeliveryInterface_Apple: error while sending: " + std::string(strerror(err)));
    }

}


void TunnelDeliveryInterface_Apple::pollMessages() {

    int received = receiveMessage(mSocketId, mReceptionBuffer, MAX_PACKET_SIZE);

    if (received > 0) {
        mLocalInterface->sendIPv6Message(mReceptionBuffer + 4, received - 4);
    }

}