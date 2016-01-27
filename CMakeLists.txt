cmake_minimum_required(VERSION 3.1)
project(MeshnetSim)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES simulator/simulator_main.cpp AbstractInterface.cpp simulator/NetworkSim.cpp simulator/SimulatorInterface.cpp Router.cpp Location.cpp Logger.cpp simulator/NetworkWidget.cpp LinkManager.cpp LinkManager.h LocalInterface.cpp LocalInterface.h simulator/ActionButton.cpp simulator/ActionButton.h simulator/draw3d.cpp simulator/draw3d.h)
add_executable(MeshnetSim ${SOURCE_FILES})

include_directories(/opt/local/include)

# I had problems with cmake just returning libfltk instead of libfltk_gl # FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
#SET(FLTK_LIBRARY_GL /opt/local/lib/libfltk_gl.dylib)
FIND_LIBRARY(FLTK_LIBRARY fltk /opt/local/lib)
FIND_LIBRARY(LIBRARY_OpenGL OpenGl)
FIND_LIBRARY(LIBRARY_glu GLU)
FIND_LIBRARY(LIBRARY_glut GLUT)

TARGET_LINK_LIBRARIES(MeshnetSim ${FLTK_LIBRARY} ${FLTK_LIBRARY_GL} ${LIBRARY_OpenGL} ${LIBRARY_glu} ${LIBRARY_glut})