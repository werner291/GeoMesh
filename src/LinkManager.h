//
// Created by Werner Kroneman on 23-01-16.
//

#ifndef MESHNETSIM_INTERFACEMANAGER_H
#define MESHNETSIM_INTERFACEMANAGER_H

#include <map>
#include "AbstractInterface.h"

class Router;

enum LinkEvent {
    LINKEVENT_CREATED,
    LINKEVENT_BROKEN
};

typedef std::function<void(std::shared_ptr<AbstractInterface>, LinkEvent)> LinkChangeListener;

/**
 * A class designed to contain a list to of all links to direct peers.
 * It also broadcasts link-related events to registered listeners.
 */
class LinkManager {

    Router *router;

    std::vector<LinkChangeListener> linkEventListeners;

public:
    LinkManager(Router *router) : router(router) { };

    std::map<int, std::shared_ptr<AbstractInterface> > mInterfaces;

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
    void connectInterface(std::shared_ptr<AbstractInterface> iFace);

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
