//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ABSTRACTINTERFACE_H
#define MESHNETSIM_ABSTRACTINTERFACE_H

#include <vector>
#include <functional>

typedef std::shared_ptr<std::vector<char> > DataBufferPtr;

class AbstractInterface;

typedef std::function<void(DataBufferPtr, int)> DataCallback;

template<typename T> T getPacketData(int charposition, DataBufferPtr data) {
    return *(reinterpret_cast<T *>(data->data() + charposition));
}

template<typename T> void setPacketData(int charposition, DataBufferPtr data, T value) {
    *(reinterpret_cast<T *>(data->data() + charposition)) = value;
}

class Router;

class AbstractInterface {

    static int nextIfaceID;

protected:
    DataCallback dataArrivedCallback;

    int iFaceID;

public:
    int getIFaceID() const {
        return iFaceID;
    }

    AbstractInterface() : iFaceID(nextIfaceID++) {

    }

    virtual bool sendData(DataBufferPtr data) = 0;

    void setDataReceivedCallback(DataCallback const &dataArrivedCallback) {
        this->dataArrivedCallback = dataArrivedCallback;
    }

};




#endif //MESHNETSIM_ABSTRACTINTERFACE_H
