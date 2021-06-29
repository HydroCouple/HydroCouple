#
# CMakeLists.txt - CMake configuration for hydrocouple header files
#
# Author: Caleb Buahin
#


cmake_minimum_required (VERSION 3.13)

project(HydroCouple 
VERSION 1.0.2 
LANGUAGES C CXX
DESCRIPTION  ""
)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

# Append local dir to module search path
# list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Sets the position independent code property for all targets
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Sets default install prefix when cmakecache is initialized for first time
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install CACHE PATH "..." FORCE)
endif()


# Define install locations (will be prepended by install prefix)
set(TOOL_DIST "bin")
set(INCLUDE_DIST "include")
set(LIBRARY_DIST "lib")
set(CONFIG_DIST  "cmake")


option(BUILD_TESTS "Build component tests (requires Boost)" OFF)
option(BUILD_DOCS "Build toolkit docs (requires Doxygen)" OFF)


# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)


# Create install target for License and Authors files
install(
    FILES
        "License.md"
    DESTINATION
        "."
)

file(GLOB_RECURSE HEADERS "src/*.h")

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(HydroCouple INTERFACE ./include)

# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_VENDOR "HydroCouple")
set(CPACK_ARCHIVE_FILE_NAME "hydrocouple")

include(CPack)
