//
// Created by System Administrator on 2/1/16.
//

#ifndef GEOMESH_UDPCONNECTIONMANAGER_H
#define GEOMESH_UDPCONNECTIONMANAGER_H

#include "../LinkManager.hpp"
#include "UDPInterface.hpp"
#include <string>
#include <string.h>

/**
 * Starts, maintains and stops UDP bridge links to other direct peers.
 */
class UDPManager {

    friend UDPInterface;

    LinkManager& linkMgr;

    int socketID;

    int localPort;

    const int MAX_FRAGMENT_SIZE = 600;

    std::map<uint16_t, std::shared_ptr<UDPInterface> > connectingLinks;

    std::map<uint16_t, std::shared_ptr<UDPInterface> > establishedLinks;

    /**
     * Take the specified message, and send it over the UDP bridge.
     * The UDPInterface is the interface that sends the message.
     * Note: You should not call this method directly, it is usually
     * called from UDPInterface.
     */
    bool sendMessage(PacketPtr message, UDPInterface* iFace);

public:
    UDPManager(LinkManager &linkMgr, int localPort);

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
     * \param localIface The interface number of the corresponding UDPInterface.
     */
    void processNormalPacketFragment(const uint8_t *buffer, int nbytes, uint16_t localIface);


    /**
     * Transform the provided {@param message} into a vector of fragments.
     *
     * @param message The message to fragment
     * @param packetNumber A 16-bit number that is incremented for each interface for each packet.
     * @param remoteIface The interface number on the other end of the UDP bridge
     *
     * @returns A vector of UDPFragmentPtrs, which all point to a fragment of the original packet.
     */
    static std::vector<UDPFragmentPtr> fragmentPacket(const PacketPtr &message, uint16_t packetNumber, uint16_t remoteIface);
};


#endif //GEOMESH_UDPCONNECTIONMANAGER_H
