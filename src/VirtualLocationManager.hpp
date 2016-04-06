//
// Created by System Administrator on 3/18/16.
//

#ifndef GEOMESH_VIRTUALLOCATIONMANAGER_H
#define GEOMESH_VIRTUALLOCATIONMANAGER_H

#include "Location.hpp"
#include <vector>

class VirtualLocationManager {

public:
    class VirtualLocationListener {
        friend VirtualLocationManager;
        virtual void virtualLocationChanged(const Location& newLocation) = 0;

    };

protected:
    std::vector<VirtualLocationListener*> listeners;

    Location loc;

public:

    VirtualLocationManager(const Location& anchorLocation) : loc(anchorLocation) {
    }

    const Location &getLocation() const {
        return loc;
    }

    void setLoc(const Location &loc, bool notifyListeners = true) {
        VirtualLocationManager::loc = loc;

        if (notifyListeners) {
            for (VirtualLocationListener *listener : listeners) {
                listener->virtualLocationChanged(loc);
            }
        }
    }

    void addListener(VirtualLocationListener* listener) {
        listeners.push_back(listener);
    }

};


#endif //GEOMESH_VIRTUALLOCATIONMANAGER_H
