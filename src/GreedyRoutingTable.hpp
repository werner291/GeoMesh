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

#ifndef GEOMESH_GREEDYROUTINGTABLE_H
#define GEOMESH_GREEDYROUTINGTABLE_H

#include "Location.hpp"
#include "AbstractLinkEndpoint.hpp"
#include "Logger.hpp"

/**
 * An entry in the greedy routing table.
 *
 * Such an entry means "Sending a packet through the interface with the specified iFaceID
 * will bring the packet closer to the target location or to a node that knows a way to get
 * the packet closer."
 */
struct GreedyRoutingTableEntry {
    Location target; // Where this routing table entry will lead.
    int iFaceID; // Widh interface to send the messages bound for the target location to.
    int hops; // An estimate on how long the path to that location is. Don't make too many assumptions on this value.
};

/**
 * A collection of GreedyRoutingTableEntries, each consisting of a target Location, an interface ID and a number of hops.
 */
class GreedyRoutingTable {

    static const double PRUNING_COEFFICIENT;

    // TODO find a better datastructure. Is a KD-tree an option?
    std::vector<GreedyRoutingTableEntry> mGreedyRoutingTable;

    std::vector<GreedyRoutingTableEntry>::iterator getClosestEntry(const Location&closestTo) {
        // How would a packet headed for the specified location be routed now?
        // Found the routing rule that most closely matches the suggestion.
        auto closestRule = GreedyRoutingTable::mGreedyRoutingTable.end();
        double closestRuleDistance = INFINITY;

        for (auto itr = GreedyRoutingTable::mGreedyRoutingTable.begin(); itr != GreedyRoutingTable::mGreedyRoutingTable.end(); itr++) {
            double distance = itr->target.distanceTo(closestTo);
            if (distance < closestRuleDistance) {
                closestRule = itr;
                closestRuleDistance = distance;
            }
        }
        
        return closestRule;
    }

public:

    /**
     * Find the routing table entry whose target location is both closer to the destination
     * than maxDistance, and closest to the destination relative to the other entries.
     *
     * \returns The interface id if one was found or -1 if no interface can be found that matches the criteria.
     */
    int getGreedyInterface(int fromInterface, const Location &destination, double maxDistance);

    bool insertIfUseful(Location suggestionTarget, int iFaceID, int hops, const Location& referenceLoc);

    /**
     *
     * \param target Relative to which target are we looking?
     *
     * \param maxDistance The entry must be closer than this.
     *
     * \return Whether there exists an entry in the table whose target is closer than maxDistance from the given
     *         location.
     */
    bool hasCloserEntry(const Location& target, double maxDistance) {
        for (auto itr = GreedyRoutingTable::mGreedyRoutingTable.begin(); itr != GreedyRoutingTable::mGreedyRoutingTable.end(); itr++) {
            if (target.distanceTo(itr->target) < maxDistance) return true;
        }
        return false;
    }

    typedef std::vector<GreedyRoutingTableEntry>::iterator iterator;
    typedef std::vector<GreedyRoutingTableEntry>::const_iterator const_iterator;

    iterator begin() {
        return mGreedyRoutingTable.begin();
    }

    iterator end() {
        return mGreedyRoutingTable.end();
    }

    const_iterator begin() const {
        return mGreedyRoutingTable.begin();
    }

    const_iterator end() const {
        return mGreedyRoutingTable.end();
    }
};


#endif //GEOMESH_GREEDYROUTINGTABLE_H
