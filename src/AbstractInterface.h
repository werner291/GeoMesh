//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ABSTRACTINTERFACE_H
#define MESHNETSIM_ABSTRACTINTERFACE_H

#include <vector>
#include <functional>
#include "constants.h"


class AbstractInterface;

typedef std::function<void(DataBufferPtr, int)> DataCallback;



class Router;

class AbstractInterface {

    static int nextIfaceID;

protected:
    DataCallback dataArrivedCallback;

    int iFaceID;

public:
    int getInterfaceId() const {
        return iFaceID;
    }

    AbstractInterface();

    virtual bool sendData(DataBufferPtr data) = 0;

    void setDataReceivedCallback(DataCallback const &dataArrivedCallback) {
        this->dataArrivedCallback = dataArrivedCallback;
    }

};




#endif //MESHNETSIM_ABSTRACTINTERFACE_H