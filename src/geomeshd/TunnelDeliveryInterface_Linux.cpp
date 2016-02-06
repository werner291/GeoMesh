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

// Define this here due to a duplicate definition when including ipv6.h
struct in6_ifreq {
         struct in6_addr ifr6_addr;
         __u32           ifr6_prefixlen;
         int             ifr6_ifindex; 
};

void TunnelDeliveryInterface_Linux::startTunnelInterface() {


        struct ifreq ifr;
        int fd, err;
        char *clonedev = "/dev/net/tun";

        /* open the clone device */
        if( (fd = open(clonedev, O_RDWR)) < 0 ) {
            int error = errno;
            Logger::log(LogLevel::ERROR, "getting utun device id " + std::string(strerror(error)));
        }

        /* preparation of the struct ifr, of type "struct ifreq" */
        memset(&ifr, 0, sizeof(ifr));

        ifr.ifr_flags = IFF_TUN | IFF_NO_PI; // This is a TUN device, and we're sending proper IP packets (so no 4 extra packets).

        /* try to create the device */
        if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
            int error = errno;
            Logger::log(LogLevel::ERROR, "getting utun device id " + std::string(strerror(error)));
            close(fd);

        }

        strcpy(iFaceName, ifr.ifr_name);
};

void TunnelDeliveryInterface_Linux::assignIP() {

    int s;
    struct ifreq ifRequest;
    // Set ifRequestOut to all 0
    memset(&ifRequest, 0, sizeof(struct ifreq));

    // Create a temporary INET6 socket
    if ((s = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        Logger::log(LogLevel::ERROR, "Error creating temporary Inet6 socket: " + std::string(strerror(errno)));
    }

    // Copy the iface name into the request
    strncpy(ifRequest.ifr_name, iFaceName, IFNAMSIZ);

    // Fetch the iface index
    if (ioctl(s, SIOCGIFINDEX, ifRequest) < 0) {
        int err = errno;
        close(s);
        Logger::log(LogLevel::ERROR, "Error SIOCGIFINDEX: " + std::string(strerror(errno)));
    }
    int ifIndex = ifRequest.ifr_ifindex;

    // Set interface up and running
    ifRequest.ifr_flags |= IFF_UP | IFF_RUNNING;
    if (ioctl(s, SIOCSIFFLAGS, ifRequest) < 0) {
        int err = errno;
        close(s);
        Logger::log(LogLevel::ERROR, "Error SIOCSIFFLAGS: " + std::string(strerror(errno)));
    }


    // The address setting
        struct in6_ifreq ifr6 = {0};

	ifr6.ifr6_ifindex = ifIndex;
        ifr6.ifr6_prefixlen = 128; // Geomesh uses the full address length. (TODO check whether this is actually such a bright idea)

        memcpy(&(ifr6.ifr6_addr), iFaceAddress.bytes, 16);

        if (ioctl(s, SIOCSIFADDR, &ifr6) < 0) {
            int err = errno;
            close(s);
            Logger::log(LogLevel::ERROR, "Error SIOCSIFADDR: " + std::string(strerror(errno)));
        }

    close(s);


}
