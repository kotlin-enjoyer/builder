cmake_minimum_required(VERSION 3.22.1)

project("builder")

add_library(${CMAKE_PROJECT_NAME} SHARED
        builder.cpp)