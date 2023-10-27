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
# Copyright: 2004-2023
#

cmake_minimum_required (VERSION 3.14)

project(
    HydroCouple 
    VERSION 2.0.0 
    LANGUAGES C CXX
    HOMEPAGE_URL "https://hydrocouple.org"
    DESCRIPTION  "HydroCouple Interface Definitions for C++ and Python"
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)


add_library(
    ${PROJECT_NAME}
    INTERFACE
    include/stdafx.h
    include/hydrocouple.h
    include/hydrocouplespatial.h
    include/hydrocouplespatialwkb.h
    include/hydrocouplespatiotemporal.h
    include/hydrocoupletemporal.h
    src/stdafx.cpp
)


target_include_directories(HydroCouple
  PUBLIC INTERFACE
  $<BUILD_INTERFACE:${HYDROCOUPLE_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:/include>
)

target_link_libraries(HydroCouple PUBLIC INTERFACE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(HydroCouple PUBLIC INTERFACE HYDROCOUPLE_LIBRARY)

install(
    TARGETS HydroCouple
    EXPORT HydroCoupleTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
 )

install(
    EXPORT HydroCoupleTargets
    FILE HydroCoupleTargets.cmake
    NAMESPACE HydroCouple::
    DESTINATION lib/cmake/HydroCouple
 )

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}Config.cmake.in
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}Library
)

install(
    FILES 
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    License.md
    DESTINATION lib/cmake/${PROJECT_NAME}
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include"
    FILES_MATCHING PATTERN "*.h"
)

add_custom_target(my_package
    COMMAND ${CMAKE_MAKE_PROGRAM} package
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    DEPENDS HydroCouple
)

include(CMakeFindDependencyMacro)
find_dependency(Qt${QT_VERSION_MAJOR})

# Configure CPack driven installer package
set(CPACK_GENERATOR "ZIP;TGZ")
set(CPACK_PACKAGE_VENDOR "HydroCouple")
set(CPACK_ARCHIVE_FILE_NAME "HydroCouple")

include(CPack)
