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
                // TODO: Send it to an output of some kind.
            } else {

                int hopsLeft = *(reinterpret_cast<int32_t *>(data->data() + TTL));

                if (hopsLeft <= 0) {
                    std::stringstream message;
                    message << "Router (" << this->uniqueaddress <<
                    "): TTL exceeded, dropping packet. " <<
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

            processRoutingSuggestion(fromIface, peerLocation, hops);

            if (hops < 1) {
                setPacketData<int32_t>(PEERINFO_HOPS, data, hops + 1);

                for (auto pair : mInterfaces) {
                    if (pair.first != fromIface)
                        pair.second->sendData(data);
                }

            }

            return true;
            break;
        }
    }

    return false;

}

void Router::processRoutingSuggestion(int fromIface, const Location &peerLocation, int hops) {

    if (hops == 1) {

        mDirectNeighbours.insert(DirectionalEntry {
            fromIface, this->mLocation.getDirectionTo(peerLocation)
        });
        Logger::log(LogLevel::DEBUG, std::to_string(mDirectNeighbours.size()));
    }

    for (auto itr = mRoutingTable.begin(); itr < mRoutingTable.end(); itr++) {
        if (itr->target.X == peerLocation.X && itr->target.Y == peerLocation.Y) {
            if (itr->hops > hops) {
                itr->iFaceID = fromIface;
                return;
            } else {
                return;
            }
        }
    }

    mRoutingTable.push_back(RoutingTableEntry {
                    peerLocation, fromIface, hops
            });
}

void Router::connectInterface(std::shared_ptr<AbstractInterface> iFace) {

    mInterfaces.insert(std::make_pair(iFace->getIFaceID(), iFace));
    iFace->setDataReceivedCallback(std::bind(&Router::handleMessage,
                                             this, std::placeholders::_1, std::placeholders::_2));

    broadcastLocationInfo();
}

void Router::broadcastLocationInfo() {

    std::shared_ptr<std::vector<char> > messageBuffer(new std::vector<char>(PEERINFO_ENTRY_UID + ADDRESS_LENGTH_OCTETS));

    *reinterpret_cast<int32_t *>(messageBuffer->data() + PROTOCOL_VERSION_LOC) = PROTOCOL_VERSION;
    *reinterpret_cast<int32_t *>(messageBuffer->data() + MESSAGE_TYPE) = MSGTYPE_PEERINFO;

    setPacketData<double>(PEERINFO_ENTRY_LOCATION_X, messageBuffer, this->mLocation.X);
    setPacketData<double>(PEERINFO_ENTRY_LOCATION_Y, messageBuffer, this->mLocation.Y);

    setPacketData<int32_t>(PEERINFO_HOPS, messageBuffer, 1);

    // TODO include UID
    //*reinterpret_cast<int64_t *>(messageBuffer->data() + PEERINFO_ENTRIES_START +
      //                           PEERINFO_ENTRY_UID) = this->uniqueaddress;

    for (auto pair : mInterfaces) {
        pair.second->sendData(messageBuffer);
    }

}

bool Router::routeGreedy(DataBufferPtr data, int fromIface, Location destination) {

    auto bestCandidate = mRoutingTable.end();
    double bestDistance = mLocation.distanceTo(destination);

    for (auto itr = mRoutingTable.begin(); itr != mRoutingTable.end(); itr++) {
        double distance = itr->target.distanceTo(destination);
        if (distance < bestDistance) {
            bestCandidate = itr;
            bestDistance = distance;
        }
    }

    if (bestCandidate == mRoutingTable.end()) {

        return false;
    }

    *(reinterpret_cast<int32_t *>(data->data() + TTL)) = getPacketData<int32_t>(TTL,data) - 1;

    sendMessageToInterface(data,bestCandidate->iFaceID);

    return true;
}

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

bool Router::canSwitchFaceToGreedy(DataBufferPtr data, Location destination) {

    double destinationDistance = destination.distanceTo(mLocation);
    double bestDistance = Location(getPacketData<double>(ROUTING_FACE_START_X, data),
                                   getPacketData<double>(ROUTING_FACE_START_Y, data)).distanceTo(destination);

    return destinationDistance < bestDistance;

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

    if (mDirectNeighbours.size() == 1) { // Just send it back.
        return sendMessageToInterface(data, fromIface);
    } else if (mDirectNeighbours.size() >= 2){ // Need to choose one

        // Need to choose a range between two directions
        auto before = mDirectNeighbours.begin();
        auto after = std::next(before); // Safe since mDirectNeighbours.size() >= 2

        while (before != mDirectNeighbours.end()) {

            bool foundRange = false;

            if (before->heading < after->heading) { // Wrapping range
                foundRange = (before->heading <= directionToDestination && directionToDestination <= after->heading);
            } else {
                foundRange = (before->heading <= directionToDestination || directionToDestination <= after->heading );
            }

            if (foundRange) {
                return sendMessageToInterface(data, routing_start_direction != FACE_ROUTE_RHL ? before->iFaceID : after->iFaceID);
            }

            before++;

            after++;
            if (after == mDirectNeighbours.end()) after = mDirectNeighbours.begin();

        }


    }

    Logger::log(LogLevel::WARN, "Could not find a suitable neighbour to relay face routing packet to.");
    return false; // Drop packet. (Better suggestions?)

}

bool Router::sendMessageToInterface(const DataBufferPtr &data, int iFace) {
    auto itr = mInterfaces.find(iFace);

    if (itr != mInterfaces.end()) {
        return itr->second->sendData(data);
    } else {
        Logger::log(LogLevel::WARN, "Trying to send data to non-existent interface " + std::to_string(iFace));
        return false;
    }
}

bool Router::routeFaceRelay(DataBufferPtr data, int fromIface, Location destination) {

    double distanceFromTarget = mLocation.distanceTo(destination);

    double searchRange = getPacketData<double>(ROUTING_FACE_RANGE, data);

    if (distanceFromTarget > searchRange) {

        setPacketData<double>(ROUTING_FACE_RANGE, data, searchRange * FACE_ROUTING_RANGE_MULTIPLIER);

        setPacketData<int32_t>(ROUTING_FACE_DIRECTION, data,
                              getPacketData<int32_t>(ROUTING_FACE_DIRECTION, data) == FACE_ROUTE_RHL ? FACE_ROUTE_LHR : FACE_ROUTE_RHL);

        return sendMessageToInterface(data, fromIface);

    } else {

        auto itr = mDirectNeighbours.begin();
        while (itr->iFaceID != fromIface) itr++;

        int DIRECTION = getPacketData<int32_t>(ROUTING_FACE_DIRECTION, data);

        // Neighbours are sorted counter-clickwise. Right-hand routing thus means selecting a previous neigbour
        if (DIRECTION == FACE_ROUTE_RHL) {
            itr++;
            if (itr == mDirectNeighbours.end()) itr = mDirectNeighbours.begin();
        } else {
            // Left hand routing, step one interface down.
            if (itr == mDirectNeighbours.begin()) itr = mDirectNeighbours.end();
            itr--;
        }

        return sendMessageToInterface(data, itr->iFaceID);
    }
}