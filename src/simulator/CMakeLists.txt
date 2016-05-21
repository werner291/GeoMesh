FIND_PACKAGE(CRYPTOPP REQUIRED)
target_link_libraries(core cryptopp)

set_property(TARGET core PROPERTY CXX_STANDARD 11)
set_property(TARGET core PROPERTY CXX_STANDARD_REQUIRED ON)

include_directories(${Boost_INCLUDE_DIRS})

FIND_LIBRARY(FLTK_LIBRARY_GL fltk_gl /opt/local/lib)
FIND_LIBRARY(FLTK_LIBRARY fltk /opt/local/lib)
FIND_LIBRARY(LIBRARY_OpenGL OpenGl)
FIND_LIBRARY(LIBRARY_glu GLU)
FIND_LIBRARY(LIBRARY_glut GLUT)

set(SIM_SRC
    simulator/NetworkSim.cpp
    simulator/NetworkWidget.cpp
    simulator/SimulatorLink.cpp
    simulator/ActionButton.cpp
    simulator/draw3d.cpp
    simulator/simulator_main.cpp)

add_executable(Simulator ${SIM_SRC})

TARGET_LINK_LIBRARIES(
    Simulator
    core
    ${FLTK_LIBRARY}
    ${FLTK_LIBRARY_GL}
    ${LIBRARY_OpenGL}
    ${LIBRARY_glu}
    ${LIBRARY_glut}
)

set_property(TARGET Simulator PROPERTY CXX_STANDARD 11)
set_property(TARGET Simulator PROPERTY CXX_STANDARD_REQUIRED ON)
