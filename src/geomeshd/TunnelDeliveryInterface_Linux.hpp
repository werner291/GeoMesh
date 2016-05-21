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

#ifndef GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
#define GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H

#include "../LocalInterface.hpp"
#include "../Packet.hpp"
#include "FileDescriptorNotifier.hpp"

#define TUNInterface_IFNAMSIZ 16

class TunnelDeliveryInterface_Linux {

    LocalInterface *mLocalInterface;

    uint8_t mReceptionBuffer[MAX_PAYLOAD_SIZE];

    char iFaceName[TUNInterface_IFNAMSIZ];
    int fd;

    Address iFaceAddress;

    void assignIP();

    void installRoute();

public:

    TunnelDeliveryInterface_Linux(LocalInterface *localInterface,
                                      const Address &iFaceAddress,
                                      FDNotifier &fdnotifier);

    void startTunnelInterface();

    void deliverIPv6Packet(uint8_t* data, size_t length);

    void pollMessages(int fd);

};


#endif //GEOMESH_TUNNELDELIVERYINTERFACE_LINUX_H
