cmake_minimum_required(VERSION 3.22)
project(psx)

add_subdirectory(vendors/spdlog)

set(CMAKE_CXX_STANDARD 20)

list(APPEND sources src/main.cpp src/psx.cpp src/cpu.cpp)

add_executable(psx ${sources})
target_link_libraries(psx spdlog)
target_include_directories(psx PRIVATE include)