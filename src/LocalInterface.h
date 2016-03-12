//
// Created by Werner Kroneman on 24-01-16.
//

#ifndef MESHNETSIM_LOCALINTERFACE_H
#define MESHNETSIM_LOCALINTERFACE_H

#include <iostream>

#include "AbstractInterface.h"
#include "Location.h"
#include "UniqueAddress.h"

/**
 * An intermediary between the Router and the system-specific delivery interface.
 *
 * Proposal: Use this as a superclass for the delivery interface?
 */
class LocalInterface {

    Router *router;
    std::function<void(PacketPtr)> dataReceivedHandler;

public:
    void setDataReceivedHandler(const std::function<void(PacketPtr)> &dataReceivedHandler) {
        LocalInterface::dataReceivedHandler = dataReceivedHandler;
    }

    LocalInterface(Router *router) : router(router) { }

    /**
     * Creates a new GeoMesh packet, copies ipV6packet into it, and writes the GeoMesh header fields in front of it.
     *
     * @param ipV6packet A valid ipV6packet
     * @bytes The length of the data
     */
    bool sendIPv6Message(const uint8_t *ipV6packet, int bytes);

    /**
     *
     */
    void dataReceived(PacketPtr data);

};


#endif //MESHNETSIM_LOCALINTERFACE_H
