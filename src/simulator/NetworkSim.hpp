//
// Created by Werner Kroneman on 22-01-16.
//

#ifndef MESHNETSIM_NETWORKSIM_H
#define MESHNETSIM_NETWORKSIM_H

#include "../Router.hpp"
#include "SimulatorLink.hpp"
#include <memory>
#include <random>
#include <vector>

class NetworkWidget;

struct SimulatedPacket {
  PacketPtr data;

  float linkProgress;
  enum { A, B } direction;
};

struct Link {

  Link(std::shared_ptr<SimulatorLink> a, std::shared_ptr<SimulatorLink> b)
      : a(a), b(b) {}

  std::shared_ptr<SimulatorLink> a;
  std::shared_ptr<SimulatorLink> b;
  std::vector<SimulatedPacket> packetsOnLine;
  time_t length;
};

struct SimulatedNode {
  std::shared_ptr<Router> router;

  inline const Location &getLocation() const {
    return router->getLocationMgr().getLocation();
  }
};

class NetworkSim {

  std::random_device rdev;
  std::mt19937 rgen;
  std::uniform_int_distribution<char> addrgen;

  int packetsSent = 0;
  int packetsReceived = 0;

  float scale;

  std::vector<SimulatedNode> nodes;
  std::vector<Link> links;

  // void createRelayHubNetwork(int numNodes, int fieldSizeX, int fieldSizeY);

  void handlePacketArrival(int messageType, Address from, Location fromLocation,
                           uint8_t *message, size_t messageSize);

public:
  float getScale() const { return scale; }

  void setScale(float scale) { NetworkSim::scale = scale; }

  NetworkSim() : rgen(rdev()), addrgen(0, 255) {}

  const std::vector<SimulatedNode> &getNodes() const { return nodes; }

  std::vector<Link> const &getLinks() const { return links; }

  void setLinks(std::vector<Link> const &links) { NetworkSim::links = links; }

  bool sendMessage(std::string message, int startNodeID, int endNodeID);

  bool createLongitudinalGridNetwork(int nodesPerRing, int rings);

  void updateSimulation(long timeDeltaMilliseconds);

  void linkRouters(std::shared_ptr<Router> &a, std::shared_ptr<Router> &b);

  void clearNetwork();

  std::vector<char> generateAddress();

  std::shared_ptr<Router> newNodeAt(double posX, double posY);

  void createConcentricNetwork(int xMax, int yMax, int rings, int ringSize);

  void networkFromOSM(std::string xmlFilePath);
};

#endif // MESHNETSIM_NETWORKSIM_H
