cmake_minimum_required(VERSION 3.16)
project(wordle)

set(CMAKE_CXX_STANDARD 14)
SET(GCC_COVERAGE_COMPILE_FLAGS "-ggdb")
include_directories(.)

add_executable(wordle
        classes.hpp
        wordle.cpp)
