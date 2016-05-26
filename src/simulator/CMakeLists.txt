
find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

set(SIM_SRC
    NetworkSim.cpp
    NetworkWidget.cpp
    SimulatorLink.cpp
    ActionButton.cpp
    simulator_main.cpp)

add_executable(Simulator ${SIM_SRC})

add_executable(VirtualLocationTest
        VirtualLocationTest_main.cpp
        VirtualLocationNetwork.cpp
        VirtualLocationNode.cpp
        vtkNetworkAdapter.cpp)

TARGET_LINK_LIBRARIES(
        Simulator
        core
        ${VTK_LIBRARIES}
)

TARGET_LINK_LIBRARIES(
        VirtualLocationTest
        core
        ${VTK_LIBRARIES}
)

set_property(TARGET Simulator PROPERTY CXX_STANDARD 11)
set_property(TARGET Simulator PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET VirtualLocationTest PROPERTY CXX_STANDARD 11)
set_property(TARGET VirtualLocationTest PROPERTY CXX_STANDARD_REQUIRED ON)
