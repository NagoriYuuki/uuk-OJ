cmake_minimum_required(VERSION 3.10)
project(uukOJ)

set(CMAKE_CXX_STANDARD 17)

include_directories(Crow/include)
include_directories(include)

file(GLOB SOURCES "src/*.cpp" "main.cpp")

add_executable(uukOJ ${SOURCES})
