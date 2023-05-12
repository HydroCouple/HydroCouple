#
# CMakeLists.txt - CMake configuration for hydrocouple header files
#
# Author: Caleb Buahin
#
# This file is part of HydroCouple.
# 
# HydroCouple is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# HydroCouple is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with HydroCouple. If not, see <http://www.gnu.org/licenses/>.
# 
# Copyright: 

cmake_minimum_required (VERSION 3.14)

project(HydroCouple 
VERSION 2.0.0 
LANGUAGES C CXX
DESCRIPTION  "HydroCouple Interface Definitions for C++ and Python"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 Qt6 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

qt_standard_project_setup()

# option(BUILD_TESTS "Build component tests (requires Boost)" OFF)
# option(BUILD_DOCS "Build toolkit docs (requires Doxygen)" OFF)


# Create install rules for vcruntime.dll, msvcp.dll, vcomp.dll etc.
set(CMAKE_INSTALL_OPENMP_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)


# Create install target for License and Authors files
install(
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    FILES
        "License.md"
    DESTINATION
        "."
)

file(GLOB_RECURSE HEADERS "include/*.h")

add_library
    ${PROJECT_NAME} 
    INTERFACE
    ${HEADERS}
)

target_include_directories(HydroCouple INTERFACE ./include)

# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_VENDOR "HydroCouple")
set(CPACK_ARCHIVE_FILE_NAME "hydrocouple")

include(CPack)
