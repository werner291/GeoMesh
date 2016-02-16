//
// Created by System Administrator on 2/2/16.
//

#include "TunnelDeliveryInterface_Linux.h"

#include "../Logger.h"

#include <regex>
#include <libexplain/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "UnixSocketsFunctions.h"

TunnelDeliveryInterface_Linux::TunnelDeliveryInterface_Linux(LocalInterface *localInterface,
                                                             const Address &iFaceAddress)
        : mLocalInterface(localInterface), iFaceAddress(iFaceAddress), fd(-1) {

    iFaceName[0] = 't';
    iFaceName[1] = 'u';
    iFaceName[2] = 'n';
    iFaceName[3] = 'G';
    iFaceName[4] = 'e';
    iFaceName[5] = 'o';
    iFaceName[6] = 0;


    //memset(iFaceName, 0, TUNInterface_IFNAMSIZ);

    localInterface->setDataReceivedHandler(
            std::bind(&TunnelDeliveryInterface_Linux::deliverIPv6Packet, this, std::placeholders::_1));
}

void TunnelDeliveryInterface_Linux::startTunnelInterface() {

    struct ifreq ifr;
    int err;
    int flags = IFF_TUN | IFF_NO_PI;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("Opening /dev/net/tun");
    }

    memset(&ifr, 0, sizeof(ifr));

    if (*iFaceName) {
        strncpy(ifr.ifr_name, iFaceName, IFNAMSIZ);
    }

    ifr.ifr_flags = flags;

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        Logger::log(LogLevel::ERROR, "Error setting tunnel flags: " + std::string(strerror(errno)));
        close(fd);
    }

    long flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);

    strcpy(iFaceName, ifr.ifr_name);


    assignIP();

    installRoute();
};

void TunnelDeliveryInterface_Linux::assignIP() {

    std::regex utunReg("tun[a-zA-Z]+");

    // Prevent a nasty bash injection under the root user.
    if (!std::regex_match(std::string(iFaceName), utunReg)) {
        Logger::log(LogLevel::ERROR, "Invalid interface name while assigning address " + std::string(iFaceName));
        return;
    };

    std::stringstream command;

    command << "ip link set " << iFaceName << " up" << std::endl;
    command << "ip -6 addr add " << iFaceAddress.toString() << " dev " << iFaceName << std::endl;

    Logger::log(LogLevel::INFO, "Assigned IPv6 address " + command.str());

    system(command.str().c_str());
}

void TunnelDeliveryInterface_Linux::pollMessages() {

    int received = read(fd, mReceptionBuffer, MAX_PAYLOAD_SIZE);

    if (received > 0) {
        mLocalInterface->sendIPv6Message(mReceptionBuffer, received);
    }
}

void TunnelDeliveryInterface_Linux::installRoute() {

    std::regex utunReg("tun[a-zA-Z]+");

    // Prevent a nasty bash injection under the root user.
    if (!std::regex_match(std::string(iFaceName), utunReg)) {
        Logger::log(LogLevel::ERROR, "Invalid interface name while installing route " + std::string(iFaceName));
        return;
    };

    system(("ip -6 route add fcf4::/16 dev " + std::string(iFaceName)).c_str());

    Logger::log(LogLevel::INFO,
                "Installed route. All traffic to fcfd:/16 will be sent through " + std::string(iFaceName));

};

void TunnelDeliveryInterface_Linux::deliverIPv6Packet(PacketPtr packet) {

    // Send to the local system.
    int result = send(fd,
                      packet->getPayload(),
                      packet->getPayloadLength(),
                      0);

    if (result == -1) {
        int err = errno;
        Logger::log(LogLevel::ERROR,
                    "TunnelDeliveryInterface_Linux: error while sending: " + std::string(strerror(err)));
    }

}
