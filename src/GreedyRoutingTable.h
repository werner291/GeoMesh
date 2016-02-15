//
// Created by System Administrator on 2/15/16.
//

#ifndef GEOMESH_GREEDYROUTINGTABLE_H
#define GEOMESH_GREEDYROUTINGTABLE_H

#include "Location.h"
#include "AbstractInterface.h"
#include "Logger.h"

struct GreedyRoutingTableEntry {
    Location target;
    int iFaceID;
    int hops;
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
    
    int getGreedyInterface(int fromInterface, const Location &destination, double maxDistance);
    
    bool insertIfUseful(Location suggestionTarget, int iFaceID, int hops, const Location& referenceLoc) {
        
        if (mGreedyRoutingTable.empty()) {
            mGreedyRoutingTable.push_back(GreedyRoutingTableEntry {
                    suggestionTarget, iFaceID, hops
            });

            Logger::log(LogLevel::DEBUG, "Added routing rule to empty primary routing table, location "
                                         + suggestionTarget.getDescription() + " will be routed to interface "
                                         + std::to_string(iFaceID) + " routing cost " + std::to_string(hops)
                                         + " hops.");

            return true;
        }

        auto closestEntry = getClosestEntry(referenceLoc);
        
        double bestRuleDistance = closestEntry->target.distanceTo(suggestionTarget);
        double suggestionDistanceFromMe = referenceLoc.distanceTo(suggestionTarget);

        // TODO change the pruning coefficient in acoordance to memory usage and availability
        if (bestRuleDistance > suggestionDistanceFromMe * PRUNING_COEFFICIENT) {

            mGreedyRoutingTable.push_back(GreedyRoutingTableEntry {
                    suggestionTarget, iFaceID, hops
            });

            Logger::log(LogLevel::DEBUG, "Added routing rule to non-empty primary routing table, location "
                                         + suggestionTarget.getDescription() + " will be routed to interface "
                                         + std::to_string(iFaceID) + " routing cost " + std::to_string(hops)
                                         + " hops.");

            return true;
        }

        return false;
    }

    bool hasCloserEntry(const Location& target, double maxDistance) {
        for (auto itr = GreedyRoutingTable::mGreedyRoutingTable.begin(); itr != GreedyRoutingTable::mGreedyRoutingTable.end(); itr++) {
            if (target.distanceTo(itr->target) < maxDistance) return true;
        }
        return false;
    }
};


#endif //GEOMESH_GREEDYROUTINGTABLE_H
