cmake_minimum_required(VERSION 3.0)

project(GeoMesh)

option(BUILD_SIMULATOR "build_simulator" OFF)

add_subdirectory(src)
add_subdirectory(test)