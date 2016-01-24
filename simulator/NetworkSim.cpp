//
// Created by Werner Kroneman on 22-01-16.
//
#include <iostream>
#include <random>
#include "NetworkSim.h"
#include "../Location.h"
#include "../Router.h"
#include "../constants.h"

void NetworkSim::updateSimulation(long timeDeltaMilliseconds) {

    for (Link& link : links) {
        for (auto itr = link.packetsOnLine.begin(); itr != link.packetsOnLine.end();) {
            itr->linkProgress += timeDeltaMilliseconds;

            if (itr->linkProgress > link.length) {

                std::shared_ptr<SimulatorInterface> router = (itr->direction == SimulatedPacket::A ? link.a : link.b);

                router->handlePacketArrival(itr->data);

                itr = link.packetsOnLine.erase(itr);
            } else {
                itr++;
            }
        }

        while (link.a->hasNextInSendQueue()) {
            SimulatedPacket packet;

            packet.data = link.a->pullNextInSendQueue();
            packet.direction = SimulatedPacket::B;
            packet.linkProgress = 0;

            link.packetsOnLine.emplace_back(packet);
        }

        while (link.b->hasNextInSendQueue()) {
            SimulatedPacket packet;

            packet.data = link.b->pullNextInSendQueue();
            packet.direction = SimulatedPacket::A;
            packet.linkProgress = 0;

            link.packetsOnLine.emplace_back(packet);
        }
    }



}

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
    return std::max(lower, std::min(n, upper));
}

void NetworkSim::createRandomNetwork(int numNodes, int fieldSizeX, int fieldSizeY) {

    nodes.clear();
    links.clear();

    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> displacement(-20,20);

    std::uniform_int_distribution<int> link(0,5);

    int routerID = 0;

    int spacing = 50;

    int cols = fieldSizeX / spacing;
    int rows = fieldSizeY / spacing;

    for (int x=0; x < cols; x ++) {
        for (int y=0; y < rows; y++ ) {

            std::shared_ptr<Router> newRouter(new Router(routerID++,
                                                         Location(x*spacing + spacing/2 + displacement(rgen),
                                                                  y*spacing + spacing/2 + displacement(rgen))));

            nodes.push_back(newRouter);

            if (x > 0 && link(rgen) != 0) {
                linkRouters(newRouter, nodes[(x-1) * rows + y]);
            }
            if (y > 0 && link(rgen) != 0) {
                linkRouters(newRouter, nodes[x * rows + y - 1]);
            }
        }
    }
}

void NetworkSim::linkRouters(std::shared_ptr<Router> &a, std::shared_ptr<Router> &b) {
    Link lnk(std::__1::make_shared<SimulatorInterface>(a.get()),
             std::__1::make_shared<SimulatorInterface>(b.get()));

    lnk.a->getRouter()->connectInterface(lnk.a);
    lnk.b->getRouter()->connectInterface(lnk.b);

    lnk.length = lnk.a->getRouter()->getLocation().distanceTo(lnk.b->getRouter()->getLocation());

    links.emplace_back(lnk);
}

bool NetworkSim::sendMessage(std::string message, int startNodeID, int endNodeID) {

    if (message.length() > 512 - PAYLOAD_START) {
        return false;
    }

    DataBufferPtr msg(new std::vector<char>(512));

    std::shared_ptr<Router> begin = this->nodes[startNodeID];
    std::shared_ptr<Router> end = this->nodes[endNodeID];

    *reinterpret_cast<int32_t*>(msg->data() + PROTOCOL_VERSION_LOC) = 0;
    *reinterpret_cast<int32_t*>(msg->data() + MESSAGE_TYPE) = MSGTYPE_PAYLOAD;
    *reinterpret_cast<double*>(msg->data() + LOCATION_COORDINATE_X) = end->getLocation().X;
    *reinterpret_cast<double*>(msg->data() + LOCATION_COORDINATE_Y) = end->getLocation().Y;
    *reinterpret_cast<int32_t*>(msg->data() + DESTINATION_ID) = endNodeID;
    *reinterpret_cast<int32_t*>(msg->data() + TTL) = 100;

    std::memcpy(msg->data() + PAYLOAD_START, message.c_str(), message.length());



    begin->handleMessage(msg,0);

    return true;
}
