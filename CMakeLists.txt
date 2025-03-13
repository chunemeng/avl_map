cmake_minimum_required(VERSION 3.22.1)

project(AVL_MAP)

set(CMAKE_CXX_STANDARD 23)
add_subdirectory(src)

OPTION(DEBUG "Enable debug" ON)

if (DEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_BUILD_TYPE DEBUG)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    set(CMAKE_BUILD_TYPE RELEASE)
endif ()


include(${CMAKE_SOURCE_DIR}/extern/gtest.cmake)


