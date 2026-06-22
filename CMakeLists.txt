cmake_minimum_required(VERSION 3.10)
project(Comp-o-Tron_6000 VERSION 1.0 LANGUAGES CXX)

# Set C++ standard and compile flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -g -O2")
set(TOOLSDIR ${CMAKE_SOURCE_DIR}/tools)

add_subdirectory(tools)
add_subdirectory(gui)
