cmake_minimum_required(VERSION 3.1)
project(MeshnetSim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES simulator/simulator_main.cpp AbstractInterface.cpp simulator/NetworkSim.cpp SimulatorInterface.cpp Router.cpp Location.cpp Logger.cpp simulator/NetworkWidget.cpp LinkManager.cpp LinkManager.h LocalInterface.cpp LocalInterface.h)
add_executable(MeshnetSim ${SOURCE_FILES})

include_directories(/opt/local/include)

FIND_LIBRARY(FLTK_LIBRARY fltk /opt/local/lib)
TARGET_LINK_LIBRARIES(MeshnetSim ${FLTK_LIBRARY})