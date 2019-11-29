cmake_minimum_required(VERSION 3.7)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
project(ShaderToy CXX C)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_BINARY_DIR})

#
# Dependencies
#

# File2CPP
include(cmake/file2cpp.cmake)

# QT
find_package(Qt4 REQUIRED)
set(QT_USE_QTOPENGL TRUE)
include(${QT_USE_FILE})

# OpenGL
add_definitions(-DGL_GLEXT_PROTOTYPES=1) # what does this do?

if (POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW)
endif(POLICY CMP0072)

find_package(OpenGL 4.3 REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# GLMath
find_package(GLM REQUIRED)

set(LIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/lib)
add_subdirectory(src)
