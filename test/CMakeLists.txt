cmake_minimum_required(VERSION 3.0)
project(Test)

include(CTest)
enable_testing()

add_subdirectory(googletest)

set(test_SRCS Location_test.cpp Packet_test.cpp Address_test.cpp UDPBridge_test.cpp ContactsSetTest.cpp DHTTest.cpp)

add_executable(runUnitTests gtest.cpp ${test_SRCS})

set_property(TARGET runUnitTests PROPERTY CXX_STANDARD 11)
set_property(TARGET runUnitTests PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(runUnitTests gtest gmock core daemon_core)
