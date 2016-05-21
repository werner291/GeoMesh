//
// Created by System Administrator on 2/2/16.
//

#include "TunnelDeliveryInterface_Linux.hpp"

#include "../Logger.hpp"

#include <regex>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "UnixSocketsFunctions.hpp"
#include "FileDescriptorNotifier.hpp"

using namespace std;

TunnelDeliveryInterface_Linux::TunnelDeliveryInterface_Linux(
        LocalInterface *localInterface,
        const Address &iFaceAddress,
        FDNotifier &fdnotifier)
        : mLocalInterface(localInterface), fd(-1), iFaceAddress(iFaceAddress)
{

    iFaceName[0] = 't';
    iFaceName[1] = 'u';
    iFaceName[2] = 'n';
    iFaceName[3] = 'G';
    iFaceName[4] = 'e';
    iFaceName[5] = 'o';
    iFaceName[6] = 0;

    //memset(iFaceName, 0, TUNInterface_IFNAMSIZ);

    localInterface->setDataReceivedHandler(
            bind(&TunnelDeliveryInterface_Linux::deliverIPv6Packet,
                 this,
                 placeholders::_1,
                 placeholders::_2
                ));

    startTunnelInterface();

    fdnotifier.addFileDescriptor(
            mSocketId,
            bind(&TunnelDeliveryInterface_Linux::pollMessages,
                 this,
                 placeholders::_1
                )
                                );
}

void TunnelDeliveryInterface_Linux::startTunnelInterface()
{

    Logger::log(LogLevel::DEBUG,
                "Trying to start tun."
               );

    struct ifreq ifr;
    int err;
    int flags = IFF_TUN | IFF_NO_PI;

    if ((fd = open("/dev/net/tun", O_RDWR)) < 0)
    {
        perror("Opening /dev/net/tun");
    }

    memset(&ifr, 0, sizeof(ifr));

    if (*iFaceName)
    {
        strncpy(ifr.ifr_name, iFaceName, IFNAMSIZ);
    }

    ifr.ifr_flags = flags;

    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0)
    {
        Logger::log(LogLevel::ERROR,
                    "Error setting tunnel flags: " + string(strerror(errno)));

        close(fd);

        if (err == EPERM)
        {
            Logger::log(LogLevel::ERROR,
                        "Most systems require admin rights to affect networking."
                                " Try running geomeshd with sudo."
                       );
            exit(EXIT_FAILURE);
        }
    }

    long flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);

    strcpy(iFaceName, ifr.ifr_name);

    assignIP();

    installRoute();

    Logger::log(LogLevel::DEBUG, "TUN setup done.");
};

void TunnelDeliveryInterface_Linux::assignIP()
{

    regex utunReg("tun[a-zA-Z]+");

    // Prevent a nasty bash injection under the root user.
    if (!regex_match(string(iFaceName), utunReg))
    {
        Logger::log(LogLevel::ERROR,
                    "Invalid interface name while assigning address "
                    + string(iFaceName));
        return;
    };

    stringstream command;

    command << "ip link set " << iFaceName << " up" << endl;
    command << "ip -6 addr add " << iFaceAddress.toString() << " dev "
    << iFaceName << endl;
    command << "ip link set dev " << iFaceName << " mtu " << MAX_PAYLOAD_SIZE;

    Logger::log(LogLevel::INFO,
                "Assigned IPv6 address " + iFaceAddress.toString());

    if (system(command.str().c_str()) != 0)
    {
        Logger::log(LogLevel::ERROR,
                    "Error while assigning address to TUN device. Exiting."
                   );
        exit(EXIT_FAILURE);
    }
}

void TunnelDeliveryInterface_Linux::pollMessages(int fd)
{
    int received = read(fd, mReceptionBuffer, MAX_PAYLOAD_SIZE);

    if (received > 0)
    {
        mLocalInterface->sendIPv6Message(mReceptionBuffer, received);
    }
}

void TunnelDeliveryInterface_Linux::installRoute()
{

    regex utunReg("tun[a-zA-Z]+");

    // Prevent a nasty bash injection under the root user.
    if (!regex_match(string(iFaceName), utunReg))
    {
        Logger::log(LogLevel::ERROR,
                    "Invalid interface name while installing route " +
                    string(iFaceName));
        return;
    };

    system(("ip -6 route add fcf4::/16 dev " + string(iFaceName)).c_str());

    Logger::log(LogLevel::INFO,
                "Installed route. All traffic to fcfd:/16 will be sent through " +
                string(iFaceName));

};

void TunnelDeliveryInterface_Linux::deliverIPv6Packet(uint8_t *data,
                                                      size_t length)
{

    // Send to the local system.
    int result = write(fd,
                       data,
                       length
                      );

    if (result == -1)
    {
        int err = errno;
        Logger::log(LogLevel::ERROR,
                    "TunnelDeliveryInterface_Linux: error while sending: " +
                    string(strerror(err)));
    }

}
