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

#ifndef MESHNETSIM_ROUTER_H
#define MESHNETSIM_ROUTER_H

#include "Location.hpp"
#include "AbstractLinkEndpoint.hpp"
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

/**
 * The Router is the class that, given a packet, decides where to send it next.
 * 
 * If the packet is addressed to the local system, it is passed to the
 * localHandler.
 *
 * If the packet is addressed to an address in the local routing table,
 * redirect it accoring to local routing rules.
 *
 * Otherwise, route using geographic routing rules.
 *
 * The router only routes, it does not manage the routing tables.
 */
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
