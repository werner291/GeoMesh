cmake_minimum_required(VERSION 3.1)
project(MeshnetSim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


include_directories(/opt/local/include)

set(CORE_SRC AbstractInterface.cpp Router.cpp Location.cpp Logger.cpp LinkManager.cpp LocalInterface.cpp
        UniqueAddress.cpp geomeshd/TunnelDeliveryInterface_Apple.cpp
        PacketFunctions.cpp geomeshd/UDPManager.cpp)
add_library(core ${CORE_SRC})

# I had problems with cmake just returning libfltk instead of libfltk_gl # FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
#SET(FLTK_LIBRARY_GL /opt/local/lib/libfltk_gl.dylib)
FIND_LIBRARY(FLTK_LIBRARY fltk /opt/local/lib)
FIND_LIBRARY(LIBRARY_OpenGL OpenGl)
FIND_LIBRARY(LIBRARY_glu GLU)
FIND_LIBRARY(LIBRARY_glut GLUT)

set(SIM_SRC simulator/NetworkSim.cpp simulator/NetworkWidget.cpp simulator/SimulatorLink.cpp simulator/ActionButton.cpp
        simulator/draw3d.cpp simulator/simulator_main.cpp)

add_executable(Simulator ${SIM_SRC})
TARGET_LINK_LIBRARIES(Simulator core ${FLTK_LIBRARY} ${FLTK_LIBRARY_GL} ${LIBRARY_OpenGL} ${LIBRARY_glu} ${LIBRARY_glut})

FIND_LIBRARY(LIBRARY_asio boost_asio)

set(DAEMON_SRC geomeshd/GeoMeshd_main.cpp geomeshd/TunnelDeliveryInterface_Apple.cpp geomeshd/UDPInterface.cpp
        geomeshd/UDPManager.cpp)
add_executable(geomeshd ${DAEMON_SRC})

TARGET_LINK_LIBRARIES(geomeshd core)# LIBRARY_asio)


