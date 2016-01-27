//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_NETWORKSIM_H
#define MESHNETSIM_NETWORKSIM_H

#include <vector>
#include <memory>
#include "SimulatorInterface.h"
#include <random>

class NetworkWidget;

struct SimulatedPacket {
    DataBufferPtr data;

    float linkProgress;
    enum {
        A,B
    } direction;
};

struct Link {

    Link(std::shared_ptr<SimulatorInterface> a, std::shared_ptr<SimulatorInterface> b) : a(a), b(b) {}

    std::shared_ptr<SimulatorInterface> a;
    std::shared_ptr<SimulatorInterface> b;
    std::vector<SimulatedPacket> packetsOnLine;
    time_t length;
};

class NetworkSim {

    std::random_device rdev;
    std::mt19937 rgen;
    std::uniform_int_distribution<char> addrgen;

public:
    float getScale() const {
        return scale;
    }

    void setScale(float scale) {
        NetworkSim::scale = scale;
    }

private:
    float scale;

public:

    NetworkSim() : rgen(rdev()), addrgen(0, 255) {}

    std::vector<std::shared_ptr<Router>> const &getNodes() const {
        return nodes;
    }

    void setNodes(std::vector<std::shared_ptr<Router>> const &nodes) {
        NetworkSim::nodes = nodes;
    }

    std::vector<Link> const &getLinks() const {
        return links;
    }

    void setLinks(std::vector<Link> const &links) {
        NetworkSim::links = links;
    }

    bool sendMessage(std::string message, int startNodeID, int endNodeID);

private:
    std::vector<std::shared_ptr<Router> > nodes;
    std::vector<Link> links;

    //void createRelayHubNetwork(int numNodes, int fieldSizeX, int fieldSizeY);

public:

    bool createLongitudinalGridNetwork(int nodesPerRing, int rings);

    void updateSimulation(long timeDeltaMilliseconds);

    void linkRouters(std::shared_ptr<Router> &a, std::shared_ptr<Router> &b);

    void clearNetwork();

    std::vector<char> generateAddress();

    std::shared_ptr<Router> newNodeAt(double posX, double posY);

    void createConcentricNetwork(int xMax, int yMax, int rings, int ringSize);
};




#endif //MESHNETSIM_NETWORKSIM_H
