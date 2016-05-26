/*
 * (c) Copyright 2016 Werner Kroneman
 *
 * This file is part of GeoMesh.
 * 
 * GeoMesh is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GeoMesh is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESHNETSIM_LOCALINTERFACE_H
#define MESHNETSIM_LOCALINTERFACE_H

#include <iostream>

#include "LocalPacketHandler.hpp"
#include "LocationLookupManager.hpp"
#include "GPSLocation.hpp"
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

  void locationLookupCompleted(Address addr, GPSLocation loc, time_t expires);

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
  void dataReceived(int messageType, Address from, GPSLocation fromLocation,
                    uint8_t *message, size_t messageSize);
};

#endif // MESHNETSIM_LOCALINTERFACE_H
