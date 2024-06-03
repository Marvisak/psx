cmake_minimum_required(VERSION 3.27)
project(psx)

set(CMAKE_CXX_STANDARD 20)

list(APPEND sources src/main.cpp src/psx.cpp src/cpu.cpp)

add_executable(psx ${sources})
target_include_directories(psx PRIVATE include)