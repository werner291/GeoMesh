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

class LinkManager {

    Router *router;


    std::vector<LinkChangeListener> linkEventListeners;

public:
    LinkManager(Router *router) : router(router) { };

    std::map<int, std::shared_ptr<AbstractInterface> > mInterfaces;

    bool sendPacket(const PacketPtr &data, int fromIface);

    void connectInterface(std::shared_ptr<AbstractInterface> iFace);

    void broadcastMessage(const PacketPtr &data);

    void addLinkListener(const LinkChangeListener &changeListener) {
        linkEventListeners.push_back(changeListener);
    }
};


#endif //MESHNETSIM_INTERFACEMANAGER_H
