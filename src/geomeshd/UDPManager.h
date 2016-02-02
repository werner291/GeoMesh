//
// Created by System Administrator on 2/1/16.
//

#ifndef GEOMESH_UDPCONNECTIONMANAGER_H
#define GEOMESH_UDPCONNECTIONMANAGER_H

#include "../LinkManager.h"
#include "UDPInterface.h"
#include <string>

class UDPManager {

    LinkManager* linkMgr;

    int socketID;

    std::vector<std::shared_ptr<UDPInterface> > connectingLinks;

    std::vector<std::shared_ptr<UDPInterface> > establishedLinks;


public:
    UDPManager(LinkManager *linkMgr);

    void connectTo(std::string address, int port);

    void pollMessages();

};


#endif //GEOMESH_UDPCONNECTIONMANAGER_H
