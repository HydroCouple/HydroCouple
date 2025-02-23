#
# CMakeLists.txt - CMake configuration for hydrocouple header files
#
# Author: Caleb Buahin (caleb.buahin@gmail.com)
#
# This file is part of HydroCouple.
#
# You can redistribute it and/or modify it under the terms of the
# MIT License as published by the Free Software Foundation.
# This file and its associated files is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.(see <http://www.gnu.org/licenses/> for details)
# 
# You should have received a copy of the MIT License
# along with HydroCouple. If not, see <http://www.gnu.org/licenses/>.
# 
# Copyright: 2004-2025
#

cmake_minimum_required (VERSION 3.14)

project(
    HydroCouple
    VERSION 2.0.0
    LANGUAGES C CXX
    HOMEPAGE_URL "https://hydrocouple.org"
    DESCRIPTION  "HydroCouple Interface Definitions for C++"
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
    ${PROJECT_NAME}
    INTERFACE
    include/version.h
    include/hydrocouple.h
    include/hydrocouplespatial.h
    include/hydrocouplespatialwkb.h
    include/hydrocouplespatiotemporal.h
    include/hydrocoupletemporal.h
)

target_include_directories(${PROJECT_NAME}
    PUBLIC INTERFACE
    $<BUILD_INTERFACE:${HYDROCOUPLE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:/include>
)

target_compile_definitions(HydroCouple PUBLIC INTERFACE HYDROCOUPLE_LIBRARY)

string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC")
set(BUILD_DATE "${BUILD_TIMESTAMP}")

# Get the commit hash
execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Get the branch name
execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(
    UUID BUILD_ID 
    NAMESPACE "00000000-0000-0000-0000-000000000000" 
    NAME "https://www.hydrocouple.org" 
    TYPE MD5
)

set(BUILD_ID "${BUILD_ID}")
set(BUILD_BRANCH "${GIT_BRANCH_NAME}")
set(BUILD_COMMIT "${GIT_COMMIT_HASH}")

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in 
    ${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
)

install(
    FILES 
        License.md
        README.md
    DESTINATION .
)

install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
 )

 
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
 )

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
   ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
   ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
   INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    License.md
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
    DESTINATION "include"
    # include citations markdown file
    FILES_MATCHING 
    PATTERN "*.md"
    PATTERN "*.cff"
    PATTERN "*.bib"
    PATTERN "*.txt"
    PATTERN "*.h"
)

# Configure CPack driven installer package
set(CPACK_GENERATOR "TGZ")
set(CPACK_PACKAGE_VENDOR "HydroCouple")
set(CPACK_ARCHIVE_FILE_NAME "HydroCouple")

include(CPack)
