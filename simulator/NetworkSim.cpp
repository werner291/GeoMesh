//
// Created by Werner Kroneman on 22-01-16.
//
#include <iostream>
#include <algorithm>
#include "NetworkSim.h"
#include "../Location.h"
#include "../Router.h"
#include "../constants.h"
#include "../Logger.h"

int cnt = 0;

void NetworkSim::updateSimulation(long timeDeltaMilliseconds) {

    for (Link& link : links) {
        for (auto itr = link.packetsOnLine.begin(); itr != link.packetsOnLine.end();) {
            itr->linkProgress += timeDeltaMilliseconds * 100000;

            if (itr->linkProgress > link.length) {

                std::shared_ptr<SimulatorInterface> router = (itr->direction == SimulatedPacket::A ? link.a : link.b);

                router->handlePacketArrival(itr->data);

                itr = link.packetsOnLine.erase(itr);
            } else {
                itr++;
            }
        }

        // Not a while loop such that only one packet is taken each cycle to simulate a non-infinite-capacity line
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

std::shared_ptr<Router> NetworkSim::newNodeAt(double posX, double posY) {

    std::shared_ptr<Router> newRouter(new Router(generateAddress(), Location(posY, posX)));

    nodes.push_back(newRouter);

    return newRouter;
}

std::vector<char> NetworkSim::generateAddress() {
    std::__1::vector<char> addr(ADDRESS_LENGTH_OCTETS);
    for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
            addr[i] = addrgen(rgen);
        }
    return addr;
}

void NetworkSim::clearNetwork() {
    nodes.clear();
    links.clear();
}

void NetworkSim::linkRouters(std::shared_ptr<Router> &a, std::shared_ptr<Router> &b) {
    Link lnk(std::__1::make_shared<SimulatorInterface>(a.get()),
             std::__1::make_shared<SimulatorInterface>(b.get()));

    lnk.a->getRouter()->getLinkManager()->connectInterface(lnk.a);
    lnk.b->getRouter()->getLinkManager()->connectInterface(lnk.b);

    lnk.length = lnk.a->getRouter()->getLocation().distanceTo(lnk.b->getRouter()->getLocation());

    links.emplace_back(lnk);
}

bool NetworkSim::sendMessage(std::string message, int startNodeID, int endNodeID) {

    std::shared_ptr<Router> begin = this->nodes[startNodeID];
    std::shared_ptr<Router> end = this->nodes[endNodeID];

    begin->getLocalIface()->sendMessage(end->getAddress(),
                                        std::make_shared<std::vector<char> >(message.begin(), message.end()),
                                        end->getLocation());

    return true;
}

bool NetworkSim::createLongitudinalGridNetwork(int nodesPerRing, int rings) {

    clearNetwork();

    for (int i = 0; i < nodesPerRing; i++) {

        newNodeAt(-180 + 360 * i / nodesPerRing, 0);

        if (i >= 1) linkRouters(nodes[i], nodes[i - 1]);

    }

    linkRouters(nodes.back(), nodes.front());

}