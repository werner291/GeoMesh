//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_NETWORKSIM_H
#define MESHNETSIM_NETWORKSIM_H

#include <vector>
#include <memory>
#include "SimulatorInterface.h"

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


public:
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

public:
    void createRandomNetwork(int numNodes, int fieldSizeX, int fieldSizeY);

    void updateSimulation(long timeDeltaMilliseconds);

    void linkRouters(std::shared_ptr<Router> &a, std::shared_ptr<Router> &b);
};




#endif //MESHNETSIM_NETWORKSIM_H
