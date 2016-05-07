/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

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
