//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ROUTER_H
#define MESHNETSIM_ROUTER_H

#include "Location.h"
#include "AbstractInterface.h"
#include "constants.h"
#include "LocalInterface.h"
#include "LinkManager.h"

#include <set>
#include <queue>

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

    std::vector<char> uniqueaddress;

private:
    Location mLocation;

    LocalInterface *localIface;

    LinkManager *linkMgr;

    std::set<DirectionalEntry> mFaceRoutingTable; // TODO combine "interfaces" and "direct neighbours"

    std::vector<RoutingTableEntry> mGreedyRoutingTable;

    std::queue<DataBufferPtr> routingQueue;

public:
    LocalInterface *getLocalIface() const {
        return localIface;
    }

    const std::vector<RoutingTableEntry> &getRoutingTable() {
        return mGreedyRoutingTable;
    }

    LinkManager *getLinkManager() const {
        return linkMgr;
    }

    const std::vector<char> &getAddress() {
        return uniqueaddress;
    }

    // TODO make the allocation of the local interface and link manager a bit more elegant
    Router(std::vector<char> uniqueaddress, Location location) : uniqueaddress(uniqueaddress), mLocation(location),
                                                                 localIface(new LocalInterface(this)),
                                                                 linkMgr(new LinkManager(this)) { };

    ~Router() {
        delete localIface;
        delete linkMgr;
    }

    bool handleMessage(std::shared_ptr<std::vector<char> > data, int fromIface);

    const Location& getLocation() const {
        return mLocation;
    }

    const int getNumNeighbours() const {
        return mFaceRoutingTable.size();
    }

    void sendLocationInfo(int iFace);

    bool processRoutingSuggestion(int fromIface, const Location &peerLocation, int hops);

    bool routeGreedy(DataBufferPtr data, int fromIface, Location destination);

    int getGreedyInterface(int fromInterface, const Location &destination);

    void greedyToFace(DataBufferPtr data);

    bool routeFaceBegin(DataBufferPtr data, int fromIface, Location destination);

    bool canSwitchFaceToGreedy(DataBufferPtr data, Location destination);

    bool routeFaceRelay(DataBufferPtr data, int fromIface, Location destination);


};


#endif //MESHNETSIM_ROUTER_H
