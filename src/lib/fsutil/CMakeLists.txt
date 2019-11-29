cmake_minimum_required(VERSION 3.7)
set(TARGET_NAME libfsutil)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB SOURCES *.cpp)
add_library(${TARGET_NAME} ${SOURCES})
