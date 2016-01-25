//
// Created by Werner Kroneman on 24-01-16.
//

#ifndef MESHNETSIM_LOCALINTERFACE_H
#define MESHNETSIM_LOCALINTERFACE_H

#include <iostream>

#include "constants.h"
#include "AbstractInterface.h"
#include "Location.h"

class LocalInterface {

    Router *router;

public:
    LocalInterface(Router *router) : router(router) { }

    bool sendMessage(const std::vector<char> &address, DataBufferPtr data, Location recipientLocation);

    void dataReceived(DataBufferPtr data) {
        //std::cout << "Received data!" << std::endl; // TODO
    }

};


#endif //MESHNETSIM_LOCALINTERFACE_H
