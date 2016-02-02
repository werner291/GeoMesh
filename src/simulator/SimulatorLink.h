//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_SIMULATORINTERFACE_H
#define MESHNETSIM_SIMULATORINTERFACE_H


#include <queue>
#include "../AbstractInterface.h"

class SimulatorLink : public AbstractInterface {

public:
    Router *getRouter() const {
        return router;
    }

private:
    Router* router;

    std::queue<std::shared_ptr<std::vector<char> > > sendQueue;

public:

    SimulatorLink(Router *router) : router(router) {
    }

    virtual bool sendData(std::shared_ptr<std::vector<char> > data);

    bool hasNextInSendQueue();

    std::shared_ptr<std::vector<char> > pullNextInSendQueue();

    void handlePacketArrival(DataBufferPtr data) {
        this->dataArrivedCallback(data, this->iFaceID);
    }
};


#endif //MESHNETSIM_SIMULATORINTERFACE_H
