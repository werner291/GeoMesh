cmake_minimum_required(VERSION 3.1)
project(MeshnetSim)

include_directories(/opt/local/include)

set(CORE_SRC 
    AbstractInterface.cpp 
    Router.cpp Location.cpp
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
    GreedyRoutingTable.hpp
    GreedyRoutingTable.cpp
    my_htonll.cpp
    VirtualLocationManager.cpp
    LocalPacketHandler.hpp
    LocalPacketHandler.cpp
    AbstractInterface.hpp
    Router.hpp
    Location.hpp
    Logger.hpp
    LinkManager.hpp 
    ContactsSet.cpp
    ContactsSet.hpp
    LocationLookupManager.hpp
    my_htonll.hpp
    Crypto.h Crypto.cpp
    Scheduler.cpp Scheduler.hpp)

add_library(core ${CORE_SRC})

set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED ON)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
find_package(Boost COMPONENTS program_options REQUIRED) 

    include_directories(${Boost_INCLUDE_DIRS}) 



IF (BUILD_SIMULATOR)
    FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
    FIND_LIBRARY(FLTK_LIBRARY fltk /opt/local/lib)
    FIND_LIBRARY(LIBRARY_OpenGL OpenGl)
    FIND_LIBRARY(LIBRARY_glu GLU)
    FIND_LIBRARY(LIBRARY_glut GLUT)

    set(SIM_SRC simulator/NetworkSim.cpp simulator/NetworkWidget.cpp simulator/SimulatorLink.cpp simulator/ActionButton.cpp
            simulator/draw3d.cpp simulator/simulator_main.cpp)

    add_executable(Simulator ${SIM_SRC})
    TARGET_LINK_LIBRARIES(Simulator core ${FLTK_LIBRARY} ${FLTK_LIBRARY_GL} ${LIBRARY_OpenGL} ${LIBRARY_glu} ${LIBRARY_glut})

    set_property(TARGET Simulator PROPERTY CXX_STANDARD 11)
    set_property(TARGET Simulator PROPERTY CXX_STANDARD_REQUIRED ON)
ENDIF ()


set(DAEMON_SRC geomeshd/GeoMeshd_main.cpp geomeshd/UDPInterface.cpp geomeshd/UDPManager.cpp geomeshd/UnixSocketsFunctions.cpp
        geomeshd/UDPFragment.cpp geomeshd/UDPReceptionBuffer.cpp)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DAEMON_SRC ${DAEMON_SRC} geomeshd/TunnelDeliveryInterface_Apple.cpp)
ENDIF ()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DAEMON_SRC ${DAEMON_SRC} geomeshd/TunnelDeliveryInterface_Linux.cpp geomeshd/UDPFragment.cpp geomeshd/UDPReceptionBuffer.cpp)
ENDIF ()

add_library(daemon_core ${DAEMON_SRC})

TARGET_LINK_LIBRARIES(daemon_core core ${Boost_LIBRARIES})

add_executable(geomeshd geomeshd/GeoMeshd_main.cpp)

set_property(TARGET daemon_core PROPERTY CXX_STANDARD 11)
set_property(TARGET daemon_core PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET geomeshd PROPERTY CXX_STANDARD 11)
set_property(TARGET geomeshd PROPERTY CXX_STANDARD_REQUIRED ON)

TARGET_LINK_LIBRARIES(geomeshd core daemon_core )
