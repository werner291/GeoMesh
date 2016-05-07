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

#ifndef MESHNETSIM_INTERFACEMANAGER_H
#define MESHNETSIM_INTERFACEMANAGER_H

#include <map>
#include "AbstractLinkEndpoint.hpp"

class Router;

enum LinkEvent {
    LINKEVENT_CREATED,
    LINKEVENT_BROKEN
};

typedef std::function<void(std::shared_ptr<AbstractLinkEndpoint>, LinkEvent)> LinkChangeListener;

/**
 * A class designed to contain a list to of all links to direct peers.
 * It also broadcasts link-related events to registered listeners.
 */
class LinkManager {

    std::vector<LinkChangeListener> linkEventListeners;

    std::function< void (const PacketPtr& packet, int fromIface) > routeInboundPacket;

public:
    LinkManager(std::function< void (const PacketPtr& packet, int fromIface) > routeInboundPacket)
      : routeInboundPacket(routeInboundPacket) { };

    std::map<int, std::shared_ptr<AbstractLinkEndpoint> > mInterfaces;

    /**
     * Send a valid GeoMesh packet through the interface with the given ID.
     *
     * \param data A pointer to a valid GeoMesh packet.
     * \param toIface The interface ID to send the packet into.
     */
    bool sendPacket(const PacketPtr &data, int toIface);

    /**
     * Register a new interface with the LinkManager. All listening parties
     * will be notified.
     *
     * Please note that Router will immediately send a location information macket.
     */
    void connectInterface(std::shared_ptr<AbstractLinkEndpoint> iFace);

    /**
     * Send a packet of data to all known links / interfaces.
     */
    void broadcastMessage(const PacketPtr &data);

    /**
     * Register a link listener.
     */
    void addLinkListener(const LinkChangeListener &changeListener) {
        linkEventListeners.push_back(changeListener);
    }
};


#endif //MESHNETSIM_INTERFACEMANAGER_H
