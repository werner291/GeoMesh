cmake_minimum_required(VERSION 3.0)
project(Test)

include(CTest)
enable_testing()

add_subdirectory(googletest)

# Core-only tests

set(core_test_SRCS CryptoTest.cpp Location_test.cpp Packet_test.cpp Address_test.cpp ContactsSetTest.cpp DHTTest.cpp Scheduler_test.cpp)

add_executable(runCoreTests gtest.cpp ${core_test_SRCS})
target_link_libraries(runCoreTests gtest gmock core )

set_property(TARGET runCoreTests PROPERTY CXX_STANDARD 11)
set_property(TARGET runCoreTests PROPERTY CXX_STANDARD_REQUIRED ON)

# Full tests

set(daemon_test_SRCS UDPBridge_test.cpp)

add_executable(runFullTests gtest.cpp ${core_test_SRCS} ${daemon_test_SRCS})
target_link_libraries(runFullTests gtest gmock core daemon_core)

set_property(TARGET runFullTests PROPERTY CXX_STANDARD 11)
set_property(TARGET runFullTests PROPERTY CXX_STANDARD_REQUIRED ON)
