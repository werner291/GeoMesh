//
// Created by Werner Kroneman on 23-01-16.
//

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
