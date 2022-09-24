cmake_minimum_required(VERSION 3.22)
project(MableEmulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(Servers)
include_directories(Shared)

add_subdirectory(Servers)
add_subdirectory(Shared)
