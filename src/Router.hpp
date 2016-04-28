//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_ROUTER_H
#define MESHNETSIM_ROUTER_H

#include "Location.hpp"
#include "AbstractInterface.hpp"
#include "LocalInterface.hpp"
#include "LinkManager.hpp"
#include "UniqueAddress.hpp"
#include "GreedyRoutingTable.hpp"
#include "LocalPacketHandler.hpp"

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
    VirtualLocationManager locationMgr;

private:
    LocalPacketHandler localHandler;

    LinkManager linkMgr;

    GreedyRoutingTable& greedyRoutingTable;

    std::set<DirectionalEntry> mFaceRoutingTable;

    std::map<Address, LocalRoutingTableEntry> mLocalRoutingTable;

    std::queue<PacketPtr> routingQueue;

public:

    LinkManager& getLinkManager() {
        return linkMgr;
    }

    const Address &getAddress() {
        return uniqueaddress;
    }

    Router(Address uniqueaddress, Location location, GreedyRoutingTable& greedyRoutingTable);

    ~Router() {
    }

    void sendMessageFromLocal(PacketPtr packet);

    bool handleMessage(PacketPtr data, int fromIface);

    /**
     * Call when diecided this packet won't be handles by the local host.
     */
    bool forwardPacket(PacketPtr data, int fromIface);

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

    const VirtualLocationManager &getLocationMgr() const {
        return locationMgr;
    }

    LocalPacketHandler &getLocalHandler() {
        return localHandler;
    }
};


#endif //MESHNETSIM_ROUTER_H
