//
// Created by Werner Kroneman on 22-01-16.
//

#include "Router.h"
#include "Logger.h"

#include "PacketFunctions.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <math.h>
#include "constants.h"


bool Router::handleMessage(std::shared_ptr<std::vector<char> > data, int fromIface) {

    int protocol_version = *(reinterpret_cast<int32_t *>(data->data() + PROTOCOL_VERSION_LOC));

    int messageType = *(reinterpret_cast<int32_t *>(data->data() + MESSAGE_TYPE));

    switch (messageType) {

        case MSGTYPE_PAYLOAD: {
            Location destination = Location::readFromPacket(DESTINATION_LOCATION, data);

            if (uniqueaddress.isDestinationOf(data)) {
                localIface->dataReceived(data);

            } else {

                int hopsLeft = getPacketData<unsigned char>(TTL, data);

                if (hopsLeft <= 0) {
                    std::stringstream message;
                    message << "TTL exceeded, dropping packet destined to: " <<
                    destination.getDescription();

                    Logger::log(LogLevel::WARN, message.str());

                    return false;
                }

                int routingMode = getPacketData<int32_t>(ROUTING_MODE, data);

                switch (routingMode) {

                    case ROUTING_GREEDY: {

                        bool greedySucceeded = routeGreedy(data, fromIface, destination);

                        if (!greedySucceeded) {
                            routeFaceBegin(data, fromIface, destination);
                        }
                    }
                        break;
                    case ROUTING_FACE: {

                        if (canSwitchFaceToGreedy(data, destination)) {
                            setPacketData(ROUTING_MODE, data, ROUTING_GREEDY);
                            return handleMessage(data, fromIface);
                        } else {
                            return routeFaceRelay(data, fromIface, destination);
                        }

                    }
                        break;
                    default: {
                        Logger::log(ERROR, "Unknown or unimplemented routing mode: " + std::to_string(routingMode) +
                                           ", dropping packet.");
                        return false;
                    }
                        break;
                }
            }
            break;
        }
        case MSGTYPE_PEERINFO: {

            // Dangerous. TODO: use the IEEE standard for doubles.
            Location peerLocation(*(reinterpret_cast<double *>(data->data() + PEERINFO_LOCATION_LAT)),
                                  *(reinterpret_cast<double *>(data->data() + PEERINFO_LOCATION_LON)));

            int hops = getPacketData<int32_t>(PEERINFO_HOPS, data);

            bool isGoodSuggestion = processRoutingSuggestion(fromIface, peerLocation, hops);

            if (isGoodSuggestion || hops < 3) {
                setPacketData<int32_t>(PEERINFO_HOPS, data, hops + 1);

                for (auto neighbour : linkMgr->mInterfaces) {
                    if (neighbour.second->getInterfaceId() != fromIface)
                        linkMgr->sendPacket(data, neighbour.second->getInterfaceId());
                }

            }

            return true;
            break;
        }
    }

    return false;

}

bool Router::processRoutingSuggestion(int fromIface, const Location &peerLocation, int hops) {

    if (hops == 1) {

        int before = mFaceRoutingTable.size();

        DirectionalEntry entry{
                fromIface, this->mVirtualLocation.getDirectionTo(peerLocation), peerLocation
        };

        mFaceRoutingTable.insert(entry);

        /*double totalWeight = 0;

        for (const DirectionalEntry& entry : mFaceRoutingTable) {
            totalWeight += 1 / (mRealLocation.distanceTo(entry.loc) + 0.0000001);
        }

        mVirtualLocation.lat = mRealLocation.lat * 0.5;
        mVirtualLocation.lon = mRealLocation.lon * 0.5;

        for (const DirectionalEntry& entry : mFaceRoutingTable) {
            double weight = 1 / (mRealLocation.distanceTo(entry.loc) + 0.0000001);

            mVirtualLocation.lat += (weight * entry.loc.lat / totalWeight) * 0.5;
            mVirtualLocation.lon += (weight * entry.loc.lon / totalWeight) * 0.5;
        }*/

    }

    if (mGreedyRoutingTable.empty()) {
        mGreedyRoutingTable.push_back(RoutingTableEntry {
                peerLocation, fromIface, hops
        });

        return true;
    }

    // How would a packet headed for the specified location be routed now?
    // Found the routing rule that most closely matches the suggestion.
    auto closestRule = mGreedyRoutingTable.end();
    double closestRuleDistance = INFINITY;

    for (auto itr = mGreedyRoutingTable.begin(); itr != mGreedyRoutingTable.end(); itr++) {
            double distance = itr->target.distanceTo(peerLocation);
            if (distance < closestRuleDistance) {
                closestRule = itr;
                closestRuleDistance = distance;
            }
    }

    double suggestionDistanceFromMe = mVirtualLocation.distanceTo(peerLocation);

    // Really need to improve this criterion, it even fails the equatorial ring scenario
    if (closestRuleDistance > suggestionDistanceFromMe / 5) {
        mGreedyRoutingTable.push_back(RoutingTableEntry {
                peerLocation, fromIface, hops
        });

        return true;
    }

    return false;
}

void Router::sendLocationInfo(int interface) {

    std::shared_ptr<std::vector<char> > messageBuffer(new std::vector<char>(PEERINFO_ENTRY_UID + ADDRESS_LENGTH_OCTETS));

    *reinterpret_cast<int32_t *>(messageBuffer->data() + PROTOCOL_VERSION_LOC) = PROTOCOL_VERSION;
    *reinterpret_cast<int32_t *>(messageBuffer->data() + MESSAGE_TYPE) = MSGTYPE_PEERINFO;

    setPacketData<double>(PEERINFO_LOCATION_LON, messageBuffer, this->mVirtualLocation.lon);
    setPacketData<double>(PEERINFO_LOCATION_LAT, messageBuffer, this->mVirtualLocation.lat);

    setPacketData<int32_t>(PEERINFO_HOPS, messageBuffer, 1);

    // TODO include UID
    //*reinterpret_cast<int64_t *>(messageBuffer->data() + PEERINFO_ENTRIES_START +
      //                           PEERINFO_ENTRY_UID) = this->uniqueaddress;

    linkMgr->sendPacket(messageBuffer, interface);

}

bool Router::routeGreedy(DataBufferPtr data, int fromIface, Location destination) {

    int outInteface = getGreedyInterface(fromIface, destination);

    if (outInteface != -1) {
        *(reinterpret_cast<int32_t *>(data->data() + TTL)) = getPacketData<int32_t>(TTL, data) - 1;


        linkMgr->sendPacket(data, outInteface);

        return true;
    } else {
        return false;
    }
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

bool Router::canSwitchFaceToGreedy(DataBufferPtr data, Location destination) {

    double bestDistance = getPacketData<float>(ROUTING_FACE_DISTANCE, data);

    if (destination.distanceTo(mVirtualLocation) < bestDistance) return true;

    for (auto itr = mGreedyRoutingTable.begin(); itr != mGreedyRoutingTable.end(); itr++) {
        if (destination.distanceTo(itr->target) < bestDistance) return true;
    }


    return false;

}

// TODO: Proper analysis of the correctness of this function.
bool Router::routeFaceBegin(DataBufferPtr data, int fromIface, Location destination) {

    int routing_start_direction = FACE_ROUTE_LHR;

    setPacketData<int32_t>(ROUTING_MODE, data, ROUTING_FACE);
    setPacketFaceRoutingDirection(data, routing_start_direction);
    setPacketFaceRoutingClosestDistance(data, mVirtualLocation.distanceTo(destination));
    setPacketFaceRoutingSearchRange(data, mVirtualLocation.distanceTo(destination) * FACE_ROUTING_RANGE_MULTIPLIER);

    // Direction in trigonometric space of the vector from the current node to the destination
    double directionToDestination = mVirtualLocation.getDirectionTo(destination);

    if (mFaceRoutingTable.size() == 1) { // Just send it back.
        return linkMgr->sendPacket(data, fromIface);
    } else if (mFaceRoutingTable.size() >= 2) { // Need to choose one

        // Need to choose a range between two directions
        auto before = mFaceRoutingTable.begin();
        auto after = std::next(before); // Safe since mFaceRoutingTable.size() >= 2

        while (before != mFaceRoutingTable.end()) {

            bool foundRange = false;

            if (before->heading < after->heading) { // Wrapping range
                foundRange = (before->heading <= directionToDestination && directionToDestination <= after->heading);
            } else {
                foundRange = (before->heading <= directionToDestination || directionToDestination <= after->heading );
            }

            if (foundRange) {
                return linkMgr->sendPacket(data, routing_start_direction != FACE_ROUTE_RHR ? before->iFaceID
                                                                                           : after->iFaceID);
            }

            before++;

            after++;
            if (after == mFaceRoutingTable.end()) after = mFaceRoutingTable.begin();

        }


    }

    //Logger::log(LogLevel::WARN, "Could not find a suitable neighbour to relay face routing packet to.");
    return false; // Drop packet. (Better suggestions?)

}

int Router::getGreedyInterface(int fromInterface, const Location &destination) {
    auto bestCandidate = mGreedyRoutingTable.end();
    double bestDistance = mVirtualLocation.distanceTo(destination);

    for (auto itr = mGreedyRoutingTable.begin(); itr != mGreedyRoutingTable.end(); itr++) {
        double distance = itr->target.distanceTo(destination);
        if (distance < bestDistance) {
            bestCandidate = itr;
            bestDistance = distance;
        }
    }

    if (bestCandidate == mGreedyRoutingTable.end()) {

        return -1;
    }

    return bestCandidate->iFaceID;
}

bool Router::routeFaceRelay(DataBufferPtr data, int fromIface, Location destination) {

    double distanceFromTarget = mVirtualLocation.distanceTo(destination);

    double searchRange = getPacketFaceRoutingSearchRange(data);

    int faceDirection = getPacketFaceDirection(data);

    if (distanceFromTarget > searchRange) {

        setPacketFaceRoutingSearchRange(data, searchRange * FACE_ROUTING_RANGE_MULTIPLIER);


        setPacketFaceRoutingDirection(data, faceDirection == FACE_ROUTE_RHR ? FACE_ROUTE_LHR : FACE_ROUTE_RHR);

        return linkMgr->sendPacket(data, fromIface);

    } else {

        auto itr = mFaceRoutingTable.begin();
        while (itr->iFaceID != fromIface && itr != mFaceRoutingTable.end()) itr++;
        if (itr == mFaceRoutingTable.end()) {
            Logger::log(LogLevel::WARN, "Received message from unknown direct peer.");
            return false; // Drop.
        }

        // Neighbours are sorted counter-clickwise. Right-hand routing thus means selecting a previous neigbour
        if (faceDirection == FACE_ROUTE_RHR) {
            itr++;
            if (itr == mFaceRoutingTable.end()) itr = mFaceRoutingTable.begin();
        } else {
            // Left hand routing, step one interface down.
            if (itr == mFaceRoutingTable.begin()) itr = mFaceRoutingTable.end();
            itr--;
        }

        return linkMgr->sendPacket(data, itr->iFaceID);
    }
}