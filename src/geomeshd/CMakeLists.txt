
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost COMPONENTS program_options REQUIRED)

set(
    DAEMON_SRC
    GeoMeshd_main.cpp 
    UDPManager.cpp 
    UnixSocketsFunctions.cpp
    http/HTTPServer.cpp
    http/HTTPResponse.cpp
    http/IncomingHTTPRequest.cpp
    RESTHandler.cpp
    RESTResourceAdapters.cpp
    FileDescriptorNotifier.cpp
    json/jsoncpp.cpp
)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(DAEMON_SRC ${DAEMON_SRC} TunnelDeliveryInterface_Apple.cpp)
ENDIF ()

IF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(DAEMON_SRC ${DAEMON_SRC} TunnelDeliveryInterface_Linux.cpp)
ENDIF ()

add_library(daemon_core ${DAEMON_SRC})

TARGET_LINK_LIBRARIES(daemon_core core ${Boost_LIBRARIES})

add_executable(geomeshd genconf.cpp GeoMeshd_main.cpp)

set_property(TARGET daemon_core PROPERTY CXX_STANDARD 11)
set_property(TARGET daemon_core PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET geomeshd PROPERTY CXX_STANDARD 11)
set_property(TARGET geomeshd PROPERTY CXX_STANDARD_REQUIRED ON)

TARGET_LINK_LIBRARIES(geomeshd daemon_core pthread)
