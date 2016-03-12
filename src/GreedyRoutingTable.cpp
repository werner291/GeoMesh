//
// Created by System Administrator on 2/15/16.
//

#include "GreedyRoutingTable.h"

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