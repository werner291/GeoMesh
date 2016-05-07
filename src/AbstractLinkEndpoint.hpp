/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESHNETSIM_ABSTRACTINTERFACE_H
#define MESHNETSIM_ABSTRACTINTERFACE_H

#include <vector>
#include <functional>
#include "Packet.hpp"

class AbstractLinkEndpoint;

typedef std::function<void(PacketPtr, int)> DataCallback;

class Router;

/**
 * An abstract object representing one end of a direct (level 2) link to a peer.
 */
class AbstractLinkEndpoint {

    // Every interface has an ID that is unique throughout the lifetime of the program.
    // Use of this static variable allows us to keep track of which interface IDs have been
    // allocated so far.
    static uint16_t nextIfaceID;

protected:
    DataCallback dataArrivedCallback;

    // The program-lifetime-unique interface ID.
    uint16_t iFaceID;

public:
    int getInterfaceId() const {
        return iFaceID;
    }

    AbstractLinkEndpoint();

    /**
     * Send a GeoMesh packet to the peer on the other side of the link of which
     * this interface represents an endpoint of.
     *
     * \param data A pointer to a valid GeoMesh packet.
     */
    virtual bool sendData(PacketPtr data) = 0;

    /**
     * Usually set by the LinkManager
     */
    void setDataReceivedCallback(DataCallback const &dataArrivedCallback) {
        this->dataArrivedCallback = dataArrivedCallback;
    }

    /**
     * For those who want to store this class in a sorted container
     * for a quick lookup using the interface ID as a key.
     *
     * Orders the interfaces by their interface ID.
     */
    bool operator>(const AbstractLinkEndpoint& other) const {
        return iFaceID > other.iFaceID;
    }

    /**
     * For those who want to store this class in a sorted container
     * for a quick lookup using the interface ID as a key.
     *
     * Orders the interfaces by their interface ID.
     */
    bool operator<(const AbstractLinkEndpoint& other) const {
        return iFaceID < other.iFaceID;
    }

};




#endif //MESHNETSIM_ABSTRACTINTERFACE_H
