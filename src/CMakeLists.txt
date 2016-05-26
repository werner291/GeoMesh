
# (c) Copyright 2016 Werner Kroneman
#
# This file is part of GeoMesh.
#
# GeoMesh is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# GeoMesh is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GeoMesh.  If not, see <http://www.gnu.org/licenses/>.


cmake_minimum_required(VERSION 3.1)
project(MeshnetSim)

include_directories(/opt/local/include)

set(CORE_SRC 
    AbstractLinkEndpoint.cpp 
    FragmentingLinkEndpoint.cpp 
    PacketFragment.cpp
    PacketDefragmenter.cpp
    Router.cpp
        GPSLocation.hpp
        GPSLocation.cpp
    Logger.cpp
    LinkManager.cpp
    LocalInterface.hpp
    LocalInterface.cpp
    UniqueAddress.hpp
    UniqueAddress.cpp
    Packet.hpp
    Packet.cpp
    VirtualLocationManager.hpp
    LocationLookupManager.cpp
        GeographicRoutingTable.hpp
        GeographicRoutingTable.cpp
    my_htonll.cpp
    VirtualLocationManager.cpp
    LocalPacketHandler.hpp
    LocalPacketHandler.cpp
    AbstractLinkEndpoint.hpp
    Router.hpp
    Logger.hpp
    LinkManager.hpp 
    ContactsSet.cpp
    ContactsSet.hpp
    LocationLookupManager.hpp
    my_htonll.hpp
    Scheduler.cpp Scheduler.hpp
    Crypto.h Crypto.cpp)

add_library(core ${CORE_SRC})

FIND_PACKAGE(CRYPTOPP REQUIRED)
target_link_libraries(core cryptopp)

set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS program_options REQUIRED) 

include_directories(${Boost_INCLUDE_DIRS})

IF (BUILD_SIMULATOR)
    add_subdirectory(simulator)
ENDIF ()

add_subdirectory(geomeshd)
