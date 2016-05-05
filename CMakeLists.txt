cmake_minimum_required(VERSION 3.0)

project(GeoMesh)

option(BUILD_SIMULATOR "build_simulator" OFF)
option(BUILD_TESTS "build_tests" OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall"  )

add_subdirectory(src)

IF (BUILD_TESTS)
    add_subdirectory(test)
ENDIF ()
