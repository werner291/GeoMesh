//
// Created by System Administrator on 2/1/16.
//

#ifndef GEOMESH_UDPCONNECTIONMANAGER_H
#define GEOMESH_UDPCONNECTIONMANAGER_H

#include "UDPConnectionManager.h"

class UDPConnectionManager {

    LinkManager* linkMgr;

public:
    UDPConnectionManager(LinkManager* linkMgr);

    void startListening(int port);

    void connectTo(std::string address, int port);

};


#endif //GEOMESH_UDPCONNECTIONMANAGER_H
