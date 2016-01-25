//
// Created by Werner Kroneman on 23-01-16.
//

#ifndef MESHNETSIM_INTERFACEMANAGER_H
#define MESHNETSIM_INTERFACEMANAGER_H

#include <map>
#include "AbstractInterface.h"

class Router;

class LinkManager {

    Router *router;

public:
    LinkManager(Router *router) : router(router) { };

    std::map<int, std::shared_ptr<AbstractInterface> > mInterfaces;

    bool sendPacket(const DataBufferPtr &data, int fromIface);

    void connectInterface(std::shared_ptr<AbstractInterface> iFace);

    void broadcastMessage(const DataBufferPtr &data);
};


#endif //MESHNETSIM_INTERFACEMANAGER_H
