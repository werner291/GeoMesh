//
// Created by Werner Kroneman on 22-01-16.
//

#include "Router.h"
#include "Logger.h"

#include <iostream>
#include <algorithm>
#include <sstream>
#include "constants.h"

bool Router::handleMessage(std::shared_ptr<std::vector<char> > data, int fromIface) {

    int protocol_version = *(reinterpret_cast<int32_t*>(data->data()+PROTOCOL_VERSION_LOC));

    int messageType = *(reinterpret_cast<int32_t*>(data->data()+MESSAGE_TYPE));

    switch (messageType) {

        case MSGTYPE_PAYLOAD: {
            Location destination(
                    *(reinterpret_cast<double *>(data->data() + LOCATION_COORDINATE_X)), // Dangerous. TODO: use the IEEE standard for doubles.
                    *(reinterpret_cast<double *>(data->data() + LOCATION_COORDINATE_Y))
            );
            int hopsLeft = *(reinterpret_cast<int32_t *>(data->data() + TTL));
            std::cout << hopsLeft << std::endl;
            if (hopsLeft <= 0) {
                std::stringstream message;
                message << "Router (" << this->uniqueaddress <<
                "): TTL exceeded, dropping packet. " <<
                destination.getDescription();

                Logger::log(LogLevel::WARN, message.str());

                return false;
            }

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
                std::stringstream message;
                message << "Router " << this->getLocation().getDescription() << ": No known interface that routes message closer to " <<
                destination.getDescription() << std::endl;

                message << "Known routes:";

                for (auto route : mRoutingTable) {
                    message << route.target.getDescription() << "@" << route.iFaceID;
                }

                Logger::log(LogLevel::WARN, message.str());

                return false;
            }

            std::stringstream message;
            message << "Router (" << this->uniqueaddress << "): Routing to iFace " << bestCandidate->iFaceID;

            Logger::log(LogLevel::DEBUG, message.str());

            *(reinterpret_cast<int32_t *>(data->data() + TTL)) = hopsLeft - 1;

            return mInterfaces[bestCandidate->iFaceID]->sendData(data);
            break;
        }
        case MSGTYPE_PEERINFO: {

            int numEntries = *(reinterpret_cast<int *>(data->data() + PEERINFO_NUMENTRIES_POS));

            for (int index = 0; index < numEntries; index++) {

                int entryStart = PEERINFO_ENTRIES_START + PEERINFO_ENTRY_SIZE * index;

                // Dangerous. TODO: use the IEEE standard for doubles.
                Location peerLocation(
                        *(reinterpret_cast<double *>(data->data() + entryStart + PEERINFO_ENTRY_LOCATION_X)),
                        *(reinterpret_cast<double *>(data->data() + entryStart + PEERINFO_ENTRY_LOCATION_Y)));

                mRoutingTable.push_back(RoutingTableEntry {
                        peerLocation, fromIface
                });
            }
            return true;
            break;
        }
    }

    return false;

}

void Router::connectInterface(std::shared_ptr<AbstractInterface> iFace) {

    mInterfaces.insert(std::make_pair(iFace->getIFaceID(),iFace));
    iFace->setDataReceivedCallback(std::bind(&Router::handleMessage,
                                             this, std::placeholders::_1, std::placeholders::_2));

    broadcastLocationInfo();
}

void Router::broadcastLocationInfo() {

    // Number of direct peers + this node.
    int infoEntries = 1;//mInterfaces.size() + 1;

    std::shared_ptr<std::vector<char> > messageBuffer(new std::vector<char>(PEERINFO_NUMENTRIES_POS + PEERINFO_ENTRY_SIZE * infoEntries));

    *reinterpret_cast<int32_t *>(messageBuffer->data() + PROTOCOL_VERSION_LOC) = PROTOCOL_VERSION;
    *reinterpret_cast<int32_t*>(messageBuffer->data() + MESSAGE_TYPE) = MSGTYPE_PEERINFO;
    *reinterpret_cast<int32_t*>(messageBuffer->data() + PEERINFO_NUMENTRIES_POS) = infoEntries;

    *reinterpret_cast<double*>(messageBuffer->data() + PEERINFO_ENTRIES_START + PEERINFO_ENTRY_LOCATION_X) = this->mLocation.X;
    *reinterpret_cast<double*>(messageBuffer->data() + PEERINFO_ENTRIES_START + PEERINFO_ENTRY_LOCATION_Y) = this->mLocation.Y;
    *reinterpret_cast<int64_t*>(messageBuffer->data() + PEERINFO_ENTRIES_START + PEERINFO_ENTRY_UID) = this->uniqueaddress;

    /*for (int entry=0; entry < mInterfaces.size(); entry++) {
        int entrystart = PEERINFO_ENTRIES_START + entry * PEERINFO_ENTRY_SIZE;

        *reinterpret_cast<int*>(messageBuffer->data() + entrystart + )
    }*/

    for (auto pair : mInterfaces) {
        pair.second->sendData(messageBuffer);
    }

}
