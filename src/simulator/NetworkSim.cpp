//
// Created by Werner Kroneman on 22-01-16.
//
#include "NetworkSim.hpp"
#include "../GPSLocation.hpp"
#include "../Logger.hpp"
#include "../Router.hpp"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>

int cnt = 0;

void NetworkSim::updateSimulation(long timeDeltaMilliseconds) {

  for (Link &link : links) {
    for (auto itr = link.packetsOnLine.begin();
         itr != link.packetsOnLine.end();) {
      itr->linkProgress += timeDeltaMilliseconds * 100000;

      if (itr->linkProgress > link.length) {

        std::shared_ptr<SimulatorLink> router =
            (itr->direction == SimulatedPacket::A ? link.a : link.b);

        router->handlePacketArrival(itr->data);

        itr = link.packetsOnLine.erase(itr);
      } else {
        itr++;
      }
    }

    // Not a while loop such that only one packet is taken each cycle to
    // simulate a non-infinite-capacity line
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

template <typename T> T clip(const T &n, const T &lower, const T &upper) {
  return std::max(lower, std::min(n, upper));
}

std::shared_ptr<Router> NetworkSim::newNodeAt(double lat, double lon) {

  std::shared_ptr<Router> newRouter(
      new Router(Address::generateRandom(), GPSLocation(lat, lon)));

  newRouter->getLocalHandler().addLocalPacketListener(
      std::bind(&NetworkSim::handlePacketArrival, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3,
                std::placeholders::_4, std::placeholders::_5),
      MSGTYPE_IPv6);

  nodes.push_back(SimulatedNode{newRouter});

  return newRouter;
}

std::vector<char> NetworkSim::generateAddress() {
  std::vector<char> addr(ADDRESS_LENGTH_OCTETS);
  for (int i = 0; i < ADDRESS_LENGTH_OCTETS; i++) {
    addr[i] = addrgen(rgen);
  }
  return addr;
}

void NetworkSim::clearNetwork() {
  nodes.clear();
  links.clear();
}

void NetworkSim::linkRouters(std::shared_ptr<Router> &a,
                             std::shared_ptr<Router> &b) {
  Link lnk(std::make_shared<SimulatorLink>(a.get()),
           std::make_shared<SimulatorLink>(b.get()));

  lnk.a->getRouter()->getLinkManager().connectInterface(lnk.a);
  lnk.b->getRouter()->getLinkManager().connectInterface(lnk.b);

  lnk.length = lnk.a->getRouter()->getLocationMgr().getLocation().distanceTo(
      lnk.b->getRouter()->getLocationMgr().getLocation());

  links.emplace_back(lnk);
}

bool NetworkSim::sendMessage(std::string message, int startNodeID,
                             int endNodeID) {

  std::shared_ptr<Router> begin = this->nodes[startNodeID].router;
  std::shared_ptr<Router> end = this->nodes[endNodeID].router;

  begin->getLocalHandler().sendFromLocal(
      MSGTYPE_IPv6,
      end->getAddress(),
      end->getLocationMgr().getLocation(),
      (const uint8_t*) message.data(),
      message.length()
  );

  packetsSent += 1;

  return true;
}

void NetworkSim::handlePacketArrival(int messageType, Address from,
                                     GPSLocation fromLocation, uint8_t *message,
                                     size_t messageSize) {
  packetsReceived += 1;

  Logger::log(LogLevel::INFO,
              "Received packet. Total sent: " + std::to_string(packetsSent) +
                  ", total received: " + std::to_string(packetsReceived) +
                  ", " + std::to_string(100 * packetsReceived / packetsSent) +
                  "%");
}

bool NetworkSim::createLongitudinalGridNetwork(int nodesPerRing, int rings) {

  clearNetwork();

  std::uniform_int_distribution<int> linkChooser(0, 10);

  for (int r = -rings / 2; r < (rings + 1) / 2; r++) {

    for (int i = 0; i < nodesPerRing; i++) {

      newNodeAt(-180 + 360 * i / nodesPerRing, r * 10);

      if (i >= 1 && linkChooser(rgen) > 0)
        linkRouters(nodes[nodes.size() - 1].router, nodes[nodes.size() - 2].router);

      if (r >= -rings / 2 + 1 && linkChooser(rgen) > 0)
        linkRouters(nodes[nodes.size() - 1].router,
                    nodes[nodes.size() - nodesPerRing - 1].router);
    }

    linkRouters(nodes[nodes.size() - 1].router, nodes[nodes.size() - nodesPerRing].router);
  }
  return true;
}

using namespace rapidxml;

void NetworkSim::networkFromOSM(std::string xmlFilePath) {

  std::map<long, std::shared_ptr<Router>> routers_osmnodes;

  rapidxml::file<> xmlFile(xmlFilePath.c_str()); // Default template is char
  rapidxml::xml_document<> doc;
  doc.parse<0>(xmlFile.data());

  xml_node<> *root = doc.first_node("osm");

  for (xml_node<> *node = root->first_node("node"); node;
       node = node->next_sibling("node")) {
    long osmID = atol(node->first_attribute("id")->value());
    double lat = atof(node->first_attribute("lat")->value());
    double lon = atof(node->first_attribute("lon")->value());

    routers_osmnodes.insert(std::make_pair(osmID, newNodeAt(lat, lon)));
  }

  for (xml_node<> *node = root->first_node("way"); node;
       node = node->next_sibling("way")) {

    xml_node<> *ndRef = node->first_node("nd");

    std::shared_ptr<Router> attachTo =
        routers_osmnodes[atol(ndRef->first_attribute("ref")->value())];

    for (ndRef = ndRef->next_sibling("nd"); ndRef;
         ndRef = ndRef->next_sibling("nd")) {
      std::shared_ptr<Router> next =
          routers_osmnodes[atol(ndRef->first_attribute("ref")->value())];

      linkRouters(attachTo, next);

      attachTo = next;
    }
  }

  Logger::log(LogLevel::INFO, "Loaded a network with " +
                                  std::to_string(nodes.size()) + " nodes and " +
                                  std::to_string(links.size()) + " links.");
}
