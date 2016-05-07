//
// Created by System Administrator on 2/15/16.
//

#include "GreedyRoutingTable.hpp"

const double GreedyRoutingTable::PRUNING_COEFFICIENT = 0.1;

int GreedyRoutingTable::getGreedyInterface(int fromInterface, const Location &destination, double maxDistance) {

    // A simple linear search for the closest item.
    // Perhaps I could use something lika a KD-tree to accelerate things?
    auto bestCandidate = GreedyRoutingTable::mGreedyRoutingTable.end();
    double bestDistance = maxDistance;

    for (auto itr = GreedyRoutingTable::mGreedyRoutingTable.begin(); itr != GreedyRoutingTable::mGreedyRoutingTable.end(); itr++) {
        double distance = itr->target.distanceTo(destination);
        if (distance < bestDistance) {
            bestCandidate = itr;
            bestDistance = distance;
        }
    }

    if (bestCandidate == GreedyRoutingTable::mGreedyRoutingTable.end()) {
        return -1;
    }

    return bestCandidate->iFaceID;
}

bool GreedyRoutingTable::insertIfUseful(Location suggestionTarget, int iFaceID, int hops, const Location& referenceLoc) {

    // Always insert if empty.
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

    // Find the entry whose location is closest to the suggestion
    auto closestEntry = getClosestEntry(suggestionTarget);

    // How far are they apart?
    double bestRuleDistance = closestEntry->target.distanceTo(suggestionTarget);

    // How far is the suggestion to me?
    double suggestionDistanceFromMe = referenceLoc.distanceTo(suggestionTarget);

    // TODO change the pruning coefficient in acoordance to memory usage and availability
    // Insert only if they are different enough.
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

    // The suggestion was not recorded.
    return false;
}
