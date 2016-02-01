cmake_minimum_required(VERSION 3.3)
project(Test)

include(CTest)
enable_testing()

add_subdirectory(googletest)

set(test_SRCS Location_test.cpp)

add_executable(runUnitTests gtest.cpp ${test_SRCS})
target_link_libraries(runUnitTests gtest gmock core)