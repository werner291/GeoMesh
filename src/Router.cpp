//
// Created by Werner Kroneman on 22-01-16.
//

#include "Router.hpp"
#include "LocalPacketHandler.hpp"


bool Router::handleMessage(PacketPtr data, int fromIface) {

    Logger::log(LogLevel::DEBUG, "Message received from " + data->getSourceAddress().toString()
                                 + " destined to " + data->getDestinationAddress().toString()
                                 + " of type " + std::to_string(data->getMessageType()));

    int protocol_version = data->getProtocolVersion();

    if (data->isDestination(uniqueaddress) ||
            data->getMessageType() == MSGTYPE_LOCATION_INFO) {

        if (data->getMessageType() == MSGTYPE_LOCATION_INFO) {
            Location peerLocation = data->getSourceLocation();

            int hops = data->getLocationInfoHopCount();

            bool isGoodSuggestion = processRoutingSuggestion(fromIface, data);

            if (isGoodSuggestion || hops < 3) {
                data->setLocationInfoHopCount(hops + 1);
                data->recomputeLocationInfoChecksum();

                for (auto neighbour : linkMgr.mInterfaces) {
                    if (neighbour.second->getInterfaceId() != fromIface)
                        linkMgr.sendPacket(data, neighbour.second->getInterfaceId());
                }

            }
        } else {
            localHandler.handleLocalPacket(data);
        }

        return true;
    } else {
        return forwardPacket(data, fromIface);
    }
}

bool Router::forwardPacket(PacketPtr data, int fromIface) {
    Address destAddr = data->getDestinationAddress();

    auto itr = mLocalRoutingTable.find(destAddr);

    if (itr != mLocalRoutingTable.end()) {
        Logger::log(LogLevel::DEBUG, "Address in local routetable.");

        if (linkMgr.sendPacket(data, itr->second.iFaceID)) {
            return true;
        } else {
            Logger::log(LogLevel::WARN, "Trying to route packet bound for " + destAddr.toString()
                                        + " through interface " + std::to_string(itr->second.iFaceID)
                                        + " but sendPacket(...) returned false.");
        }
        // Else, try to route according to location
    }

    Location destination = data->getDestinationLocation();

    int routingMode = data->getRoutingMode();

    switch (routingMode) {

        case ROUTING_GREEDY: {

            bool greedySucceeded = routeGreedy(data, fromIface, destination);

            if (!greedySucceeded) {
                return routeFaceBegin(data, fromIface, destination);
            }
        }
            break;
        case ROUTING_FACE_LEFT:
        case ROUTING_FACE_RIGHT: {

            if (canSwitchFaceToGreedy(data, destination)) {
                data->setRoutingMode(ROUTING_GREEDY);
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

    return false;
}

bool Router::processRoutingSuggestion(int fromIface, PacketPtr suggestionPacket) {

    // Sanity check...
    assert(suggestionPacket->getMessageType() == MSGTYPE_LOCATION_INFO);

    if (!suggestionPacket->verifyLocationInformation()) {
        return false; // The location information was damaged. (TODO add intentional falsification check)
    }

    Location peerLocation = suggestionPacket->getSourceLocation();

    int hops = suggestionPacket->getLocationInfoHopCount();

    if (hops == 1) {
        // This is a direct neighbour

        int before = mFaceRoutingTable.size();

        DirectionalEntry entry {
                fromIface, locationMgr.getLocation().getDirectionTo(peerLocation), peerLocation
        };

        mFaceRoutingTable.insert(entry);

        Logger::log(LogLevel::DEBUG, "Added face routing table entry, location " + peerLocation.getDescription()
                                     + " from iFace " + std::to_string(fromIface) + ".");
    }

    if (hops < 3) {
        auto &localEntry = mLocalRoutingTable[suggestionPacket->getSourceAddress()];

        localEntry.iFaceID = fromIface;
        localEntry.lastUpdated = time(NULL);

        Logger::log(LogLevel::DEBUG,
                    "Added peer to local routing table " + suggestionPacket->getSourceAddress().toString()
                    + " from iFace " + std::to_string(fromIface) + ".");
    }

    return greedyRoutingTable.insertIfUseful(peerLocation, fromIface, hops, locationMgr.getLocation());

}

void Router::sendLocationInfo(int interface) {

    Logger::log(LogLevel::DEBUG, "Sending location info into interface " + std::to_string(interface));


    PacketPtr locationPacker = Packet::createLocationInfoPacket(locationMgr.getLocation(), this->getAddress());
    linkMgr.sendPacket(locationPacker, interface);

}

bool Router::routeGreedy(PacketPtr data, int fromIface, Location destination) {

    int outInteface = greedyRoutingTable.getGreedyInterface(fromIface, destination,
                                                            destination.distanceTo(locationMgr.getLocation()));

    if (outInteface != -1) {

        linkMgr.sendPacket(data, outInteface);

        return true;
    } else {

        Logger::log(LogLevel::DEBUG, "Packet has hit local minimum, switching to face routing if possible.");

        return false;
    }
}

template<typename T>
int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

bool Router::canSwitchFaceToGreedy(PacketPtr data, Location destination) {

    double bestDistance = data->getPacketFaceRoutingClosestDistance();

    if (destination.distanceTo(locationMgr.getLocation()) < bestDistance) return true;

    if (greedyRoutingTable.hasCloserEntry(destination, bestDistance)) return true;

    return false;

}

bool Router::routeFaceBegin(PacketPtr data, int fromIface, Location destination) {

    data->setRoutingMode(ROUTING_FACE_RIGHT);

    data->setPacketFaceRoutingClosestDistance(locationMgr.getLocation().distanceTo(destination));

    data->setPacketFaceRoutingSearchRange(locationMgr.getLocation().distanceTo(destination) * FACE_ROUTING_RANGE_MULTIPLIER);

    // Direction in trigonometric space of the vector from the current node to the destination
    double directionToDestination = locationMgr.getLocation().getDirectionTo(destination);

    if (mFaceRoutingTable.size() == 1) { // Just send it back.
        return linkMgr.sendPacket(data, fromIface);
    } else if (mFaceRoutingTable.size() >= 2) { // Need to choose one

        // Need to choose a range between two directions
        auto before = mFaceRoutingTable.begin();
        auto after = std::next(before); // Safe since mFaceRoutingTable.size() >= 2

        while (before != mFaceRoutingTable.end()) {

            bool foundRange = false;

            if (before->heading < after->heading) { // Wrapping range
                foundRange = (before->heading <= directionToDestination && directionToDestination <= after->heading);
            } else {
                foundRange = (before->heading <= directionToDestination || directionToDestination <= after->heading);
            }

            if (foundRange) {
                return linkMgr.sendPacket(data, data->getRoutingMode() != ROUTING_FACE_RIGHT ? before->iFaceID
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

bool Router::routeFaceRelay(PacketPtr data, int fromIface, Location destination) {

    double distanceFromTarget = locationMgr.getLocation().distanceTo(destination);

    double searchRange = data->getPacketFaceRoutingSearchRange();

    int routemode = data->getRoutingMode();

    if (distanceFromTarget > searchRange) {

        data->setPacketFaceRoutingSearchRange(searchRange * FACE_ROUTING_RANGE_MULTIPLIER);

        data->setRoutingMode(routemode == ROUTING_FACE_RIGHT ? ROUTING_FACE_LEFT : ROUTING_FACE_RIGHT);

        return linkMgr.sendPacket(data, fromIface);

    } else {

        auto itr = mFaceRoutingTable.begin();
        while (itr->iFaceID != fromIface && itr != mFaceRoutingTable.end()) itr++;
        if (itr == mFaceRoutingTable.end()) {
            Logger::log(LogLevel::WARN, "Received message from unknown direct peer.");
            return false; // Drop.
        }

        // Neighbours are sorted counter-clockwise. Right-hand routing thus means selecting a previous neigbour
        if (routemode == ROUTING_FACE_RIGHT) {
            itr++;
            if (itr == mFaceRoutingTable.end()) itr = mFaceRoutingTable.begin();
        } else {
            // Left hand routing, step one interface down.
            if (itr == mFaceRoutingTable.begin()) itr = mFaceRoutingTable.end();
            itr--;
        }

        return linkMgr.sendPacket(data, itr->iFaceID);
    }
}

Router::Router(Address uniqueaddress, Location location, GreedyRoutingTable& greedyRoutingTable) :
        uniqueaddress(uniqueaddress),
        locationMgr(location),
        localHandler(locationMgr, uniqueaddress, std::bind(&Router::handleMessage, this, std::placeholders::_1, 0)),
        linkMgr(std::bind(&Router::handleMessage, this, std::placeholders::_1, std::placeholders::_2)),
        greedyRoutingTable(greedyRoutingTable) {

    linkMgr.addLinkListener([this](std::shared_ptr<AbstractInterface> iFace, LinkEvent event) {

        if (event == LINKEVENT_CREATED) {
            this->sendLocationInfo(iFace->getInterfaceId());
        }
    });

}
