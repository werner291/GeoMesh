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
#include "TunnelInterface_Apple.h"
#include "../Logger.h"

#define UTUN_CONTROL_NAME "com.apple.net.utun_control"
#define UTUN_OPT_IFNAME 2



TunnelInterface_Apple::TunnelInterface_Apple(std::weak_ptr<LocalInterface> localInterface) : mLocalInterface(localInterface) {
     localInterface.lock()->setDataReceivedHandler(std::bind(&TunnelInterface_Apple::deliverIPv6Packet, this, std::placeholders::_1));
}

void TunnelInterface_Apple::startTunnelInterface() {

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

    struct sockaddr_ctl addr;

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

    int maxNameSize = 512; // TODO too arbitrary

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


}



/**
 * Used a system() call to ifconfig. I was drowning in obscure compiler errors while trying to do it the proper ioctl() way.
 * If anyone reading this feels courageous... IMHO this works just fine
 */
void TunnelInterface_Apple::assignIP(char *iFaceName, const Address& addr) {

    std::regex utunReg("utun[0-9]+");

    // Prevent a nasty bash injection under the root user.
    if (!std::regex_match(std::string(iFaceName), utunReg)) {
        Logger::log(LogLevel::ERROR, "Invalid interface name " + std::string(iFaceName));
        return;
    };

    std::stringstream command;

    command << "ifconfig " << iFaceName << " inet6 add " << std::hex;

    for (int i = 0; i+1 < ADDRESS_LENGTH_OCTETS; i += 2) {

        if (i != 0) command << ":";

        command << (unsigned int) addr.bytes[i] << (unsigned int) addr.bytes[i+1];

    }

    command << "/64";

    Logger::log(LogLevel::INFO, "Assigned IPv6 address " + command.str());

    system(command.str().c_str());
}

void TunnelInterface_Apple::deliverIPv6Packet(DataBufferPtr packet) {

}


void TunnelInterface_Apple::pollSocket() {

    int nbytes = recvfrom(mSocketId, mReceptionBuffer, MAX_PACKET_SIZE - IPv6_START, 0, NULL, NULL);

    if (nbytes > 0) {

        mLocalInterface.lock()->sendIPv6Message(mReceptionBuffer+2, nbytes - 2);

    } else if (nbytes == 0) {

    } else {
        int err = errno;
        Logger::log(LogLevel::WARN, "Error when receiving packet " + std::string(strerror(err)));
    }


}