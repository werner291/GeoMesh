//
// Created by System Administrator on 2/1/16.
//

#ifndef GEOMESH_UDPCONNECTIONMANAGER_H
#define GEOMESH_UDPCONNECTIONMANAGER_H

#include "../LinkManager.hpp"
#include "../FragmentingLinkEndpoint.hpp"
#include "../Scheduler.hpp"
#include <string>
#include <string.h>

class UnixSocketInterface : public FragmentingLinkEndpoint {

    public:
   UnixSocketInterface(FragmentHandlerCallback handler, sockaddr_storage* addr) 
       : FragmentingLinkEndpoint(handler) {
       
       memcpy(&(this->addr),addr,sizeof(sockaddr_storage));
   }; 

   sockaddr_storage addr;
};

/**
 * Starts, maintains and stops UDP bridge links to other direct peers.
 */
class UDPManager {

    LinkManager& linkMgr;

    int socketID;

    int localPort;

    const int MAX_FRAGMENT_SIZE = 600;

    std::map<uint16_t, std::shared_ptr<UnixSocketInterface> > connectingLinks;

    std::map<uint16_t, std::shared_ptr<UnixSocketInterface> > establishedLinks;

    bool sendFragment(PacketFragmentPtr frag, uint16_t localIfaceID);

    void sendControlMessage(const std::string& msg, struct sockaddr_in& destAddr);

public:
    UDPManager(LinkManager &linkMgr, int localPort, Scheduler& scheduler);

    /**
     * Send a UDP bridge hello packet to the specified address and port,
     * and creates an interface associated to that peer. The interface is
     * stored in the "connectingLinks", and the router is NOT notified immediately.
     *
     * No assumptions should be made about the state of the link after this call.
     */
    void connectTo(std::string address, int port);

    /**
     * Polls the socket for new packets, and processes them if there are any.
     */
    void pollMessages();

    /**
     * Datagrams directed towards interface 0 need to be processed by this method.
     * They are abut the status of the UDP link itself.
     */
    void processBridgeControlMessage(char *buffer, sockaddr_in &sender);

    /**
     * Take the bytes representing a normal GeoMesh packet, and send it to the Router.
     *
     * \param buffer Pointer to the first byte of the GeoMesh packet header
     * \param nbytes Integer representing the number of bytes in the buffer.
     * \param localIface The interface number of the corresponding FragmentingLinkEndpoint.
     */
    void processNormalPacketFragment(const uint8_t *buffer, int nbytes, uint16_t localIface);

};


#endif //GEOMESH_UDPCONNECTIONMANAGER_H
