cmake_minimum_required(VERSION 3.0)

project(GeoMesh)

option(BUILD_SIMULATOR "build_simulator" OFF)
option(BUILD_TESTS "build_tests" OFF)

add_subdirectory(src)

IF (BUILD_TESTS)
    add_subdirectory(test)
ENDIF ()