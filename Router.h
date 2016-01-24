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
    int hops;
};

struct DirectionalEntry {
    int iFaceID;
    double heading;

    bool operator<(const DirectionalEntry& other) const {
        return this->heading < other.heading;
    }

    bool operator>(const DirectionalEntry& other) const {
        return this->heading > other.heading;
    }
};

class Router {

    long uniqueaddress;

private:
    Location mLocation;

    std::map<int, std::shared_ptr<AbstractInterface> > mInterfaces;

    std::set<DirectionalEntry> mDirectNeighbours; // TODO combine "interfaces" and "direct neighbours"

    std::vector<RoutingTableEntry> mRoutingTable;

public:

    Router(long uniqueaddress, Location location): uniqueaddress(uniqueaddress), mLocation(location) {};

    bool handleMessage(std::shared_ptr<std::vector<char> > data, int fromIface);

    void connectInterface(std::shared_ptr<AbstractInterface> iFace);

    const Location& getLocation() const {
        return mLocation;
    }

    const int getNumNeighbours() const {
        return mDirectNeighbours.size();
    }

    void broadcastLocationInfo();

    void processRoutingSuggestion(int fromIface, const Location &peerLocation, int hops);

    bool routeGreedy(DataBufferPtr data, int fromIface, Location destination);

    void greedyToFace(DataBufferPtr data);

    bool routeFaceBegin(DataBufferPtr data, int fromIface, Location destination);

    bool canSwitchFaceToGreedy(DataBufferPtr data, Location destination);

    bool routeFaceRelay(DataBufferPtr data, int fromIface, Location destination);

    bool sendMessageToInterface(const DataBufferPtr &data, int fromIface);
};


#endif //MESHNETSIM_ROUTER_H
