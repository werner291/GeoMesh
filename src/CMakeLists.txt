cmake_minimum_required(VERSION 3.0)
project(MeshnetSim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


include_directories(/opt/local/include)

set(CORE_SRC AbstractInterface.cpp Router.cpp Location.cpp Logger.cpp LinkManager.cpp LocalInterface.cpp
        UniqueAddress.cpp PacketFunctions.cpp geomeshd/UDPManager.cpp)
add_library(core ${CORE_SRC})

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
ENDIF ()


set(DAEMON_SRC geomeshd/GeoMeshd_main.cpp geomeshd/UDPInterface.cpp geomeshd/UDPManager.cpp)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DAEMON_SRC ${DAEMON_SRC} geomeshd/TunnelDeliveryInterface_Apple.cpp)
ENDIF ()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DAEMON_SRC ${DAEMON_SRC} geomeshd/TunnelDeliveryInterface_Linux.cpp)
ENDIF ()

add_executable(geomeshd ${DAEMON_SRC})

TARGET_LINK_LIBRARIES(geomeshd core)# LIBRARY_asio)


