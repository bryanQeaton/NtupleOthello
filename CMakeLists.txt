cmake_minimum_required(VERSION 4.0)
project(Othello)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

add_library(cereal INTERFACE)
add_library(cereal::cereal ALIAS cereal)
target_include_directories(cereal INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/third_party")
add_executable(Othello main.cpp
        position.h
        eval.h
        minmax.h
        ntuples.h
)
target_link_libraries(Othello PRIVATE cereal::cereal)

