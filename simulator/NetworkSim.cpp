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
/*
void NetworkSim::createRelayHubNetwork(int numNodes, int fieldSizeX, int fieldSizeY) {

    clearNetwork();

    std::uniform_int_distribution<int> link(0, 10);

    for (int i = 0; i < numNodes; i++) {

        newNodeAt(numNodes, fieldSizeX, fieldSizeY, i);

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

}*/

std::shared_ptr<Router> NetworkSim::newNodeAt(double posX, double posY) {

    std::shared_ptr<Router> newRouter(new Router(generateAddress(), Location(posX, posY)));

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

std::vector<std::shared_ptr<Router> > NetworkSim::createRandomGridNetwork(int xMin, int yMin, int xMax, int yMax,
                                                                          int random_displacement, int spacing) {

    xMin += random_displacement;
    yMin += random_displacement;

    xMax -= random_displacement;
    xMax -= random_displacement;

    std::uniform_int_distribution<int> displacement(-random_displacement, random_displacement);

    std::uniform_int_distribution<int> link(1, 10);

    std::uniform_int_distribution<char> addrgen(0, 255);

    int cols = (xMax - xMin) / spacing - 1;
    int rows = (yMax - yMin) / spacing - 1;

    std::vector<std::shared_ptr<Router> > gridNodes;

    for (int x=0; x < cols; x ++) {
        for (int y=0; y < rows; y++ ) {

            std::vector<char> addr(ADDRESS_LENGTH_OCTETS);
            for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
                addr[i] = addrgen(rgen);
            }

            std::shared_ptr<Router> newRouter(new Router(addr,
                                                         Location(xMin + x*spacing + displacement(rgen),
                                                                  yMin + y*spacing + displacement(rgen))));

            gridNodes.push_back(newRouter);

            if (x > 0 && link(rgen) != 0) {
                linkRouters(newRouter, gridNodes[(x-1) * rows + y]);
            }
            if (y > 0 && link(rgen) != 0) {
                linkRouters(newRouter, gridNodes[x * rows + y - 1]);
            }
        }
    }

    nodes.insert(nodes.end(), gridNodes.begin(), gridNodes.end());

    return gridNodes;
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

void NetworkSim::createIslandNetwork(int sizeX, int sizeY) {

    clearNetwork();

    std::vector< std::vector< std::shared_ptr < Router > > > subnets;

    for (int i=0; i < 3; i++) {

        double a = 2 * i * M_PI / 3;

        subnets.emplace_back(createRandomGridNetwork(cos(a) * 300 - 150 + sizeX / 2, sin(a) * 300 - 150 + sizeY / 2,
                                                     cos(a) * 300 + 150 + sizeX / 2, sin(a) * 300 + 150 + sizeY / 2, 10, 50));

        if (i >= 1) {
            linkRouters( subnets[i - 1][(subnets[i - 1].size()+1)/2 ] , subnets[i][(subnets[i].size()+1)/2 ] );
        }

    }

}

void NetworkSim::createCrumpledGridNetwork(int xMax, int yMax) {
    clearNetwork();

    createRandomGridNetwork(0,0,xMax,yMax, 1, 50);

    Logger::log(LogLevel::INFO, "Generated crumpled grid network of " + std::to_string(nodes.size()) + " nodes.");
}

void NetworkSim::createConcentricNetwork(int xMax, int yMax, int rings, int ringSize) {

    clearNetwork();

    std::shared_ptr< Router > centralNode = newNodeAt(xMax/2,yMax/2);

    for (int ring = 0; ring < rings; ring++) {

        double radius = (ring+1) * 100;

        for (int rNode = 0; rNode < ringSize; rNode++) {
            double a = 2 * rNode * M_PI / ringSize + M_PI/8;

            newNodeAt(xMax/2 + cos(a) * radius, yMax/2 + sin(a) * radius);

            if (rNode >= 1) {
                linkRouters(nodes[nodes.size()-1], nodes[nodes.size()-2]);

            }
            linkRouters(nodes.back(), ring >= 1 ? nodes[nodes.size()-ringSize-1] : centralNode);
        }
        linkRouters(nodes.back(), nodes[nodes.size()-ringSize]);

    }

}