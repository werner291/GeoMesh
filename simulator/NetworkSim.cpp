//
// Created by Werner Kroneman on 22-01-16.
//
#include <iostream>
#include <random>
#include "NetworkSim.h"
#include "../Location.h"
#include "../Router.h"
#include "../constants.h"

int cnt = 0;

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

        // Not a while loop such that only one packet is taken each cycle to simulate a non-infinite-capacity line
        if (link.a->hasNextInSendQueue()) {
            SimulatedPacket packet;

            packet.data = link.a->pullNextInSendQueue();
            packet.direction = SimulatedPacket::B;
            packet.linkProgress = 0;

            link.packetsOnLine.emplace_back(packet);
        }

        if (link.b->hasNextInSendQueue()) {
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

void NetworkSim::createRelayHubNetwork(int numNodes, int fieldSizeX, int fieldSizeY) {

    nodes.clear();
    links.clear();

    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> link(0, 10);

    std::uniform_int_distribution<char> addrgen(0, 255);

    for (int i = 0; i < numNodes; i++) {

        std::vector<char> addr(ADDRESS_LENGTH_OCTETS);
        for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
            addr[i] = addrgen(rgen);
        }

        std::shared_ptr<Router> newRouter(new Router(addr,
                                                     Location(fieldSizeX / 2 + cos(2 * M_PI * i / numNodes) * 300,
                                                              fieldSizeY / 2 + sin(2 * M_PI * i / numNodes) * 300)));

        nodes.push_back(newRouter);

        if (i >= 1) linkRouters(nodes[i], nodes[i - 1]);

    }

    for (int i = 0; i < numNodes; i++) {

        const int NUM_ON_RING = 5;

        std::vector<std::shared_ptr<Router> > ring;

        for (int j = 0; j < NUM_ON_RING; j++) {

            std::vector<char> addr(ADDRESS_LENGTH_OCTETS);
            for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
                addr[i] = addrgen(rgen);
            }

            std::shared_ptr<Router> newRouter(new Router(addr,
                                                         Location(nodes[i]->getLocation().X +
                                                                  cos(2 * M_PI * j / NUM_ON_RING) * 150,
                                                                  nodes[i]->getLocation().Y +
                                                                  sin(2 * M_PI * j / NUM_ON_RING) * 150)));

            nodes.push_back(newRouter);

            ring.push_back(newRouter);

            linkRouters(newRouter, nodes[i]);

            if (j >= 1) linkRouters(ring[j], ring[j - 1]);


        }

        linkRouters(ring.front(), ring.back());

    }

}

void NetworkSim::createRandomGridNetwork(int fieldSizeX, int fieldSizeY) {

    nodes.clear();
    links.clear();

    std::random_device rdev;
    std::mt19937 rgen(rdev());

    std::uniform_int_distribution<int> displacement(-50, 50);

    std::uniform_int_distribution<int> link(0, 10);

    std::uniform_int_distribution<char> addrgen(0, 255);

    int routerID = 0;

    int spacing = 75;

    int cols = fieldSizeX / spacing - 1;
    int rows = fieldSizeY / spacing - 1;

    for (int x=0; x < cols; x ++) {
        for (int y=0; y < rows; y++ ) {

            std::vector<char> addr(ADDRESS_LENGTH_OCTETS);
            for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
                addr[i] = addrgen(rgen);
            }

            std::shared_ptr<Router> newRouter(new Router(addr,
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
