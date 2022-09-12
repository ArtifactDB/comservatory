cmake_minimum_required(VERSION 3.14)

project(comservatory
    VERSION 1.0.0
    DESCRIPTION "C++ library for strictly validating and loading CSV files"
    LANGUAGES CXX)

add_library(comservatory INTERFACE)

target_include_directories(comservatory INTERFACE include/)

add_subdirectory(extern)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
