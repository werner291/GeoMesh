//
// Created by Werner Kroneman on 24-01-16.
//

#ifndef MESHNETSIM_LOCALINTERFACE_H
#define MESHNETSIM_LOCALINTERFACE_H

#include <iostream>

#include "LocalPacketHandler.hpp"
#include "LocationLookupManager.hpp"
#include "Location.hpp"
#include "UniqueAddress.hpp"

/**
 * An intermediary between the Router and the system-specific delivery
 * interface.
 *
 * Proposal: Use this as a superclass for the delivery interface?
 */
class LocalInterface {

  std::function<void(uint8_t *data, size_t dataSize)> dataReceivedHandler;
  LocalPacketHandler& packetHandler;
  LocationLookupManager &llm;
  std::map<Address, std::vector<std::vector<uint8_t> > > waitingForLocationLookup;

public:

  LocalInterface(LocalPacketHandler& handler,
                 LocationLookupManager& llm);

    void setDataReceivedHandler(const std::function<void(uint8_t *, size_t)> &dataReceivedHandler) {
        LocalInterface::dataReceivedHandler = dataReceivedHandler;
    }

  void locationLookupCompleted(Address addr, Location loc, time_t expires);

  /**
   * Creates a new GeoMesh packet, copies ipV6packet into it, and writes the
   * GeoMesh header fields in front of it.
   *
   * @param ipV6packet A valid ipV6packet
   * @bytes The length of the data
   */
  bool sendIPv6Message(const uint8_t *ipV6packet, int bytes);

  /**
   * An IPv6 message was received. Deliver it to the host system.
   */
  void dataReceived(int messageType, Address from, Location fromLocation,
                    uint8_t *message, size_t messageSize);
};

#endif // MESHNETSIM_LOCALINTERFACE_H
