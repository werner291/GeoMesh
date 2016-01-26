//
// Created by Werner Kroneman on 22-01-16.
//

#include "Router.h"
#include "Logger.h"

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
            Location destination(
                    *(reinterpret_cast<double *>(data->data() +
                                                 LOCATION_COORDINATE_X)), // Dangerous. TODO: use the IEEE standard for doubles.
                    *(reinterpret_cast<double *>(data->data() + LOCATION_COORDINATE_Y))
            );

            if (destination == mLocation) {
                // Received packet. TODO use UID, don't assume locations are unique.

                localIface->dataReceived(data);

            } else {

                int hopsLeft = getPacketData<int32_t>(TTL, data);

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
            Location peerLocation(
                    *(reinterpret_cast<double *>(data->data() + PEERINFO_ENTRY_LOCATION_X)),
                    *(reinterpret_cast<double *>(data->data() + PEERINFO_ENTRY_LOCATION_Y)));

            int hops = getPacketData<int32_t>(PEERINFO_HOPS, data);

            bool isGoodSuggestion = processRoutingSuggestion(fromIface, peerLocation, hops);

            if (isGoodSuggestion || hops < 3) {
                setPacketData<int32_t>(PEERINFO_HOPS, data, hops + 1);

                for (auto neighbour : linkMgr->mInterfaces) {
                    if (neighbour.second->getIFaceID() != fromIface)
                        linkMgr->sendPacket(data, neighbour.second->getIFaceID());
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
                fromIface, this->mLocation.getDirectionTo(peerLocation)
        };

        mFaceRoutingTable.insert(entry);

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

    double suggestionDistanceFromMe = mLocation.distanceTo(peerLocation);

    if (closestRuleDistance > suggestionDistanceFromMe / 2) {
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

    setPacketData<double>(PEERINFO_ENTRY_LOCATION_X, messageBuffer, this->mLocation.X);
    setPacketData<double>(PEERINFO_ENTRY_LOCATION_Y, messageBuffer, this->mLocation.Y);

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

    double bestDistance = Location(getPacketData<double>(ROUTING_FACE_START_X, data),
                                   getPacketData<double>(ROUTING_FACE_START_Y, data)).distanceTo(destination);

    if (destination.distanceTo(mLocation) < bestDistance) return true;

    for (auto itr = mGreedyRoutingTable.begin(); itr != mGreedyRoutingTable.end(); itr++) {
        if (destination.distanceTo(itr->target) < bestDistance) return true;
    }


    return false;

}

// TODO: Proper analysis of the correctness of this function.
bool Router::routeFaceBegin(DataBufferPtr data, int fromIface, Location destination) {

    int routing_start_direction = FACE_ROUTE_LHR;

    setPacketData<int32_t>(ROUTING_MODE, data, ROUTING_FACE);
    setPacketData<int32_t>(ROUTING_FACE_DIRECTION, data, routing_start_direction);
    setPacketData<double>(ROUTING_FACE_START_X, data, this->mLocation.X);
    setPacketData<double>(ROUTING_FACE_START_Y, data, this->mLocation.Y);
    setPacketData<double>(ROUTING_FACE_RANGE, data, mLocation.distanceTo(destination) * FACE_ROUTING_RANGE_MULTIPLIER);

    // Direction in trigonometric space of the vector from the current node to the destination
    double directionToDestination = mLocation.getDirectionTo(destination);

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
                return linkMgr->sendPacket(data, routing_start_direction != FACE_ROUTE_RHL ? before->iFaceID
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
    double bestDistance = mLocation.distanceTo(destination);

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

    double distanceFromTarget = mLocation.distanceTo(destination);

    double searchRange = getPacketData<double>(ROUTING_FACE_RANGE, data);

    if (distanceFromTarget > searchRange) {

        setPacketData<double>(ROUTING_FACE_RANGE, data, searchRange * FACE_ROUTING_RANGE_MULTIPLIER);

        setPacketData<int32_t>(ROUTING_FACE_DIRECTION, data,
                              getPacketData<int32_t>(ROUTING_FACE_DIRECTION, data) == FACE_ROUTE_RHL ? FACE_ROUTE_LHR : FACE_ROUTE_RHL);

        return linkMgr->sendPacket(data, fromIface);

    } else {

        auto itr = mFaceRoutingTable.begin();
        while (itr->iFaceID != fromIface && itr != mFaceRoutingTable.end()) itr++;
        if (itr == mFaceRoutingTable.end()) {
            Logger::log(LogLevel::WARN, "Received message from unknown direct peer.");
            return false; // Drop.
        }

        int DIRECTION = getPacketData<int32_t>(ROUTING_FACE_DIRECTION, data);

        // Neighbours are sorted counter-clickwise. Right-hand routing thus means selecting a previous neigbour
        if (DIRECTION == FACE_ROUTE_RHL) {
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