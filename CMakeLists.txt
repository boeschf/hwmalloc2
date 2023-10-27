cmake_minimum_required(VERSION 3.24)

# CMake auxiliary includes
include(CMakeDependentOption)
include(CheckIPOSupported)

# Read version from file
file(STRINGS "VERSION" PROJECT_VERSION)

# Extract major, minor, and patch parts
string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.(.*)" _ ${PROJECT_VERSION})
set(MAJOR_VERSION ${CMAKE_MATCH_1})
set(MINOR_VERSION ${CMAKE_MATCH_2})
set(PATCH_VERSION ${CMAKE_MATCH_3})

# Project name and version
project(hwmalloc2 VERSION ${PROJECT_VERSION} LANGUAGES CXX)

# cmake module path for find_package
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Set the output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Set the library and archive output directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Global C++ options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Effectively adds '-fpic' flag to CXX_FLAGS. Needed for dynamic catalogues.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Have LTO where possible, ie add -flto
check_ipo_supported(RESULT HAVE_LTO OUTPUT ERR_LTO)
if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    if(HAVE_LTO)
        message (STATUS "LTO support found, enabling")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(STATUS "No LTO: ${ERR_LTO}")
    endif()
endif()

# Build options including address sanitizer
get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(isMultiConfig)
    if(NOT "Asan" IN_LIST CMAKE_CONFIGURATION_TYPES)
        list(APPEND CMAKE_CONFIGURATION_TYPES Asan)
    endif()
else()
    set(allowedBuildTypes Asan Debug Release RelWithDebInfo MinSizeRel)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "${allowedBuildTypes}")
    if(CMAKE_BUILD_TYPE AND NOT CMAKE_BUILD_TYPE IN_LIST allowedBuildTypes)
        message(FATAL_ERROR "Invalid build type: ${CMAKE_BUILD_TYPE}")
    endif()
endif()

set(CMAKE_C_FLAGS_ASAN
    "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C compiler for Asan build type or configuration." FORCE)

set(CMAKE_CXX_FLAGS_ASAN
    "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer" CACHE STRING
    "Flags used by the C++ compiler for Asan build type or configuration." FORCE)

set(CMAKE_EXE_LINKER_FLAGS_ASAN
    "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker flags to be used to create executables for Asan build type." FORCE)

set(CMAKE_SHARED_LINKER_FLAGS_ASAN
    "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} -fsanitize=address" CACHE STRING
    "Linker lags to be used to create shared libraries for Asan build type." FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
endif()

# Options
set(HWMALLOC2_ENABLE_DEVICE OFF CACHE BOOL "Build with cuda/hip support")
set(HWMALLOC2_ENABLE_LOGGING OFF CACHE BOOL "Print logging info to cerr")

# Library
add_library(hwmalloc2 INTERFACE)
add_library(hwmalloc2::hwmalloc2 ALIAS hwmalloc2)
target_include_directories(hwmalloc2 INTERFACE
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>)

# Generate hwmalloc config file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/hwmalloc2/config.hpp @ONLY)

# Testing
enable_testing()
add_subdirectory(test)

# Export targets, Install rules
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS hwmalloc2
    EXPORT hwmalloc2-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES ${PROJECT_BINARY_DIR}/include/hwmalloc2/config.hpp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hwmalloc2)

install(EXPORT hwmalloc2-targets
    FILE hwmalloc2-targets.cmake
    NAMESPACE hwmalloc2::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

