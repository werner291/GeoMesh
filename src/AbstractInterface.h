//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ABSTRACTINTERFACE_H
#define MESHNETSIM_ABSTRACTINTERFACE_H

#include <vector>
#include <functional>
#include "Packet.h"


class AbstractInterface;

typedef std::function<void(PacketPtr, int)> DataCallback;

class Router;

class AbstractInterface {

    static uint16_t nextIfaceID;

protected:
    DataCallback dataArrivedCallback;

    uint16_t iFaceID;

public:
    int getInterfaceId() const {
        return iFaceID;
    }

    AbstractInterface();

    virtual bool sendData(PacketPtr data) = 0;

    void setDataReceivedCallback(DataCallback const &dataArrivedCallback) {
        this->dataArrivedCallback = dataArrivedCallback;
    }

    // For those who want to store this class in a sorted container
    bool operator>(const AbstractInterface& other) const {
        return iFaceID > other.iFaceID;
    }
    bool operator<(const AbstractInterface& other) const {
        return iFaceID < other.iFaceID;
    }

};




#endif //MESHNETSIM_ABSTRACTINTERFACE_H
