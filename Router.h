//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ROUTER_H
#define MESHNETSIM_ROUTER_H

#include "Location.h"
#include "AbstractInterface.h"

#include <set>
#include <map>

struct RoutingTableEntry {
    Location target;
    int iFaceID;

};

class Router {

    long uniqueaddress;

private:
    Location mLocation;

    std::map<int, std::shared_ptr<AbstractInterface> > mInterfaces;

    std::vector<RoutingTableEntry> mRoutingTable;

public:

    Router(long uniqueaddress, Location location): uniqueaddress(uniqueaddress), mLocation(location) {};

    bool handleMessage(std::shared_ptr<std::vector<char> > data, int fromIface);

    void connectInterface(std::shared_ptr<AbstractInterface> iFace);

    const Location& getLocation() const {
        return mLocation;
    }

    void broadcastLocationInfo();

};


#endif //MESHNETSIM_ROUTER_H
