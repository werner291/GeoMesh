//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ROUTER_H
#define MESHNETSIM_ROUTER_H

#include "Location.h"
#include "AbstractInterface.h"
#include "LocalInterface.h"
#include "LinkManager.h"
#include "UniqueAddress.h"
#include "DHTRoutingTable.h"

#include <set>
#include <queue>

struct RoutingTableEntry {
    Location target;
    int iFaceID;
    int hops;

};

struct LocalRoutingTableEntry {
    int iFaceID;
    time_t lastUpdated;
};

struct DirectionalEntry {
    int iFaceID;
    double heading;
    Location loc;

    bool operator<(const DirectionalEntry& other) const {
        return this->heading < other.heading;
    }

    bool operator>(const DirectionalEntry& other) const {
        return this->heading > other.heading;
    }
};

class Router {

    Address uniqueaddress;

private:
    Location mVirtualLocation;

    Location mRealLocation;

    LocalInterface *localIface;

    LinkManager *linkMgr;

    std::set<DirectionalEntry> mFaceRoutingTable;

    std::map<Address, LocalRoutingTableEntry> mLocalRoutingTable;

    DHTRoutingTable dhtRoutingTable;

    std::queue<PacketPtr> routingQueue;

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

    const Address &getAddress() {
        return uniqueaddress;
    }

    // TODO make the allocation of the local interface and link manager a bit more elegant
    Router(Address uniqueaddress, Location location) : uniqueaddress(uniqueaddress), mVirtualLocation(location),
                                                       mRealLocation(location),
                                                       localIface(new LocalInterface(this)),
                                                       linkMgr(new LinkManager(this)) {

        linkMgr->addLinkListener([this](std::shared_ptr<AbstractInterface> iFace, LinkEvent event) {

            if (event == LINKEVENT_CREATED) {
                this->sendLocationInfo(iFace->getInterfaceId()); // Maybe the router should decide what should be done?
            }
        });

    };

    ~Router() {
        delete localIface;
        delete linkMgr;
    }

    bool handleMessage(PacketPtr data, int fromIface);

    const Location&getVirtualLocation() const {
        return mVirtualLocation;
    }

    const int getNumNeighbours() const {
        return mFaceRoutingTable.size();
    }

    void sendLocationInfo(int iFace);

    bool processRoutingSuggestion(int fromIface, PacketPtr suggestionPacket);

    bool routeGreedy(PacketPtr data, int fromIface, Location destination);

    int getGreedyInterface(int fromInterface, const Location &destination);

    void greedyToFace(PacketPtr data);

    bool routeFaceBegin(PacketPtr data, int fromIface, Location destination);

    bool canSwitchFaceToGreedy(PacketPtr data, Location destination);

    bool routeFaceRelay(PacketPtr data, int fromIface, Location destination);

    void processDHTRoutingSuggestion(Address addr, Location loc);
};


#endif //MESHNETSIM_ROUTER_H
