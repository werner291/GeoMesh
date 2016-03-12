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
#include "GreedyRoutingTable.h"

#include <set>
#include <queue>



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

    GreedyRoutingTable greedyRoutingTable;

    std::set<DirectionalEntry> mFaceRoutingTable;

    std::map<Address, LocalRoutingTableEntry> mLocalRoutingTable;

    DHTRoutingTable dhtRoutingTable;

    std::queue<PacketPtr> routingQueue;

public:
    LocalInterface *getLocalIface() const {
        return localIface;
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



    void greedyToFace(PacketPtr data);

    bool routeFaceBegin(PacketPtr data, int fromIface, Location destination);

    /**
     * Check whether a packet that is currently in face routing mode
     * can be switched back into greedy routing mode.
     */
    bool canSwitchFaceToGreedy(PacketPtr data, Location destination);

    /**
     * Relay a packet that is currently in face routing mode. Will NOT change the routing mode.
     */
    bool routeFaceRelay(PacketPtr data, int fromIface, Location destination);

    /**
     * Check whether this information would improve the DHT routing table,
     * and insert it if so.
     */
    void processDHTRoutingSuggestion(Address addr, Location loc);
};


#endif //MESHNETSIM_ROUTER_H
