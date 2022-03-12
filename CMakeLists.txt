cmake_minimum_required(VERSION 3.16)
project(wordle)

set(CMAKE_CXX_STANDARD 20)
SET(GCC_COVERAGE_COMPILE_FLAGS "-ggdb -pthread")



include_directories(.)

add_executable(wordle
        classes.hpp
        wordle_prep.cpp classes.cpp)

find_package (Threads)
target_link_libraries (wordle ${CMAKE_THREAD_LIBS_INIT})
