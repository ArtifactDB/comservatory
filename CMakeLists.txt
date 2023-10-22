cmake_minimum_required(VERSION 3.14)

project(comservatory
    VERSION 1.1.0
    DESCRIPTION "C++ library for strictly validating and loading CSV files"
    LANGUAGES CXX)

add_library(comservatory INTERFACE)
add_library(artifactdb::comservatory ALIAS comservatory)

include(GNUInstallDirs)
target_include_directories(comservatory INTERFACE 
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_comservatory>")

option(COMSERVATORY_FETCH_EXTERN "Automatically fetch comservatory's external dependencies." ON)
if(COMSERVATORY_FETCH_EXTERN)
    add_subdirectory(extern)
else()
    find_package(ltla_byteme CONFIG REQUIRED)
endif()

target_link_libraries(comservatory INTERFACE ltla::byteme)

# Building the test-related machinery, if we are compiling this library directly.
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(COMSERVATORY_TESTS "Build comservatory's test suite." ON)
else()
    option(COMSERVATORY_TESTS "Build comservatory's test suite." OFF)
endif()

if(COMSERVATORY_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()

# Setting up the installation commands.
include(CMakePackageConfigHelpers)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/artifactdb_comservatory)

install(TARGETS comservatory 
    EXPORT comservatoryTargets)

install(EXPORT comservatoryTargets
    FILE artifactdb_comservatoryTargets.cmake
    NAMESPACE artifactdb::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_comservatory)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_comservatoryConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_comservatory)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/artifactdb_comservatoryConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/artifactdb_comservatory)
