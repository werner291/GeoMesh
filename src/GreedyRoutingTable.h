//
// Created by System Administrator on 2/15/16.
//

#ifndef GEOMESH_GREEDYROUTINGTABLE_H
#define GEOMESH_GREEDYROUTINGTABLE_H

#include "Location.h"
#include "AbstractInterface.h"
#include "Logger.h"

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

    /**
     * Inserts this suggestion into the routing table if and only if it is useful.
     * It is useful if there is no known routing rule that has a target location
     * similar to the proposal, where similarity is whether the suggested location
     * is within a certain distance from another target location, the threshold for
     * this distance itself being depedant on distance from the reference location
     * and memory usage.
     *
     * \param suggestionTarget Where the suggestion says the routing rule will go.
     * \param iFaceID Which interface to send the messages bound for the suggestionTarget to.
     * \param hops An estimate on how many hops it takes to get to suggestionTarget, usually the hop counter
     *        of the Location Info message.
     *
     * \param referenceLoc Which location to use when determining the similarity threshold, usually the location
     *        of the node that this program is running on.
     *
     * \return Whether the suggestion was inserted in the table, useful to know whether to re-broadcast
     *         the location info message.
     */
    bool insertIfUseful(Location suggestionTarget, int iFaceID, int hops, const Location& referenceLoc) {

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
};


#endif //GEOMESH_GREEDYROUTINGTABLE_H
