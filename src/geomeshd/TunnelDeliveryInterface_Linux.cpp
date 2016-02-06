//
// Created by System Administrator on 2/2/16.
//

#include "TunnelDeliveryInterface_Linux.h"

#include "../Logger.h"

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "UnixSocketsFunctions.h"

// Define this here due to a duplicate definition when including ipv6.h
struct in6_ifreq {
    struct in6_addr ifr6_addr;
    __u32 ifr6_prefixlen;
    int ifr6_ifindex;
};

TunnelDeliveryInterface_Linux::TunnelDeliveryInterface_Linux(LocalInterface *localInterface,
                                                             const Address &iFaceAddress)
        : mLocalInterface(localInterface), iFaceAddress(iFaceAddress), fd(-1) {

    iFaceName = "tunGeo";

    //memset(iFaceName, 0, TUNInterface_IFNAMSIZ);

    localInterface->setDataReceivedHandler(
            std::bind(&TunnelDeliveryInterface_Linux::deliverIPv6Packet, this, std::placeholders::_1));
}

void TunnelDeliveryInterface_Linux::startTunnelInterface() {

    struct ifreq ifr = {0};

    /* open the clone device */
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        Logger::log(LogLevel::ERROR, "Error opening clone device: " + std::string(strerror(errno)));
    }

    // This is a TUN device
    ifr.ifr_flags = IFF_TUN;

    /* try to create the device */
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
        Logger::log(LogLevel::ERROR, "Error creating tun device " + std::string(strerror(errno)));
        close(fd);
    }

    strcpy(iFaceName, ifr.ifr_name);

    Logger::log(LogLevel::INFO, "Allocated interface " + std::string(iFaceName));

    assignIP();
};

void TunnelDeliveryInterface_Linux::assignIP() {

    int s;
    int err;
    struct ifreq ifRequest = {0};

    // Create a temporary INET6 socket
    if (err = (s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        Logger::log(LogLevel::ERROR, "Error creating temporary Inet6 socket: " + std::string(strerror(err)));
    }

    // Copy the iface name into the request
    strncpy(ifRequest.ifr_name, iFaceName, IFNAMSIZ);

    Logger::log(LogLevel::ERROR, "Tun interface name: " + std::string(ifRequest.ifr_name));



    // Fetch the iface index
    if (ioctl(s, SIOCGIFINDEX, ifRequest) < 0) {
        close(s);
        Logger::log(LogLevel::ERROR, "Error SIOCGIFINDEX: " + std::string(strerror(errno)));
    }
    int ifIndex = ifRequest.ifr_ifindex;

    printf("ifIndex %i", ifIndex);

    // Set interface up and running
    ifRequest.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (err = ioctl(s, SIOCSIFFLAGS, ifRequest) < 0) {
        int err = errno;
        close(s);
        Logger::log(LogLevel::ERROR, "Error SIOCSIFFLAGS: " + std::string(strerror(err)));
    }


    // The address setting
    struct in6_ifreq ifr6 = {0};

    ifr6.ifr6_ifindex = ifIndex;
    ifr6.ifr6_prefixlen = 128; // Geomesh uses the full address length. (TODO check whether this is actually such a bright idea)

    memcpy(&(ifr6.ifr6_addr), iFaceAddress.bytes, 16);

    if (err = ioctl(s, SIOCSIFADDR, &ifr6) < 0) {
        int err = errno;
        close(s);
        Logger::log(LogLevel::ERROR, "Error SIOCSIFADDR: " + std::string(strerror(err)));
    }

    close(s);
}

void TunnelDeliveryInterface_Linux::pollMessages() {

    int received = receiveMessage(fd, mReceptionBuffer, MAX_PACKET_SIZE);

    if (received > 0) {
        mLocalInterface->sendIPv6Message(mReceptionBuffer + 4, received - 4);
    }


}

void TunnelDeliveryInterface_Linux::deliverIPv6Packet(DataBufferPtr packet) {

    //printf("Sock int: %i", fd);

    // I should create a Packet class...
    // Clear 4 octets of memory at the from of the buffer by shifting everything to the right
    packet->resize(packet->size() + 4);
    memmove(packet->data() + 4, packet->data(), 4);


    ((uint16_t *) packet->data())[0] = htons(0);            // Always 0
    ((uint16_t *) packet->data())[1] = htons(AF_INET6);   // Set to AF_INET6 so it is handled by the Internet stack.

    // Send to the local system.
    int result = send(fd,
                      packet->data(),
                      packet->size(),
                      0);
    //(struct sockaddr*) &addr,
    //sizeof(addr));

    if (result == -1) {
        int err = errno;
        Logger::log(LogLevel::ERROR,
                    "TunnelDeliveryInterface_Linux: error while sending: " + std::string(strerror(err)));
    }

}