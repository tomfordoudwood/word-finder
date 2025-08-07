cmake_minimum_required(VERSION 3.5)

project(word-finder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(word-finder src/main.cpp)

add_subdirectory(test)
