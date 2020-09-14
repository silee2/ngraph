# ******************************************************************************
# Copyright 2017-2020 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ******************************************************************************

cmake_minimum_required (VERSION 3.18)

message(STATUS "CMAKE_VERSION: ${CMAKE_VERSION}")

# set directory where the custom finders live
set(NGRAPH_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../..)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${NGRAPH_BASE_DIR}/cmake/Modules/")
get_property(GFX_GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT GFX_GENERATOR_IS_MULTI_CONFIG)
    if (CMAKE_BUILD_TYPE)
        set(RELEASE_TYPES Debug Release RelWithDebInfo MinSizeRel)
        list(FIND RELEASE_TYPES ${CMAKE_BUILD_TYPE} INDEX_FOUND)
        if (${INDEX_FOUND} EQUAL -1)
            message(FATAL_ERROR "CMAKE_BUILD_TYPE must be one of Debug, Release, RelWithDebInfo, or MinSizeRel")
        endif()
    endif()
endif()

if (UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Enable <PackageName>_ROOT for CMake 3.12+
cmake_policy(SET CMP0074 NEW)

if (APPLE)
    # Enable MACOS_RPATH by default.
    cmake_policy(SET CMP0042 NEW)
    # Enable CMAKE_<LANG>_COMPILER_ID AppleClang
    cmake_policy(SET CMP0025 NEW)
    # APPLE: Set CMAKE_OSX_SYSROOT if not set already.
    cmake_policy(SET CMP0074 NEW)
    execute_process(COMMAND sw_vers -productVersion
        OUTPUT_VARIABLE OSX_FULL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)

    string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*$" "\\1"
        OSX_SHORT_VERSION "${OSX_FULL_VERSION}")

    message(STATUS "Detected: OSX ${OSX_SHORT_VERSION}")

    if (CMAKE_OSX_SYSROOT)
        message(STATUS "Using CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT}")
    else()
        execute_process(COMMAND xcode-select -p
            OUTPUT_VARIABLE APPLE_DEV_DIR
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        if("${APPLE_DEV_DIR}" STREQUAL "/Library/Developer/CommandLineTools")
            # Command line tools only
            set(XCODE_ISYSROOT ${APPLE_DEV_DIR}/SDKs/MacOSX.sdk)
            message(STATUS "Trying command line tool sdk at ${XCODE_ISYSROOT}.")
            if(NOT EXISTS ${XCODE_ISYSROOT})
                message(FATAL_ERROR "Cannot find macos sdk.")
            endif()
        else()
            # Xcode is installed
            set(XCODE_ISYSROOT ${APPLE_DEV_DIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${OSX_SHORT_VERSION}.sdk)
            message(STATUS "Trying Xcode sdk at ${XCODE_ISYSROOT}.")
            if(NOT EXISTS ${XCODE_ISYSROOT})
                set(XCODE_ISYSROOT ${APPLE_DEV_DIR}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk)
                if(NOT EXISTS ${XCODE_ISYSROOT})
                    message(FATAL_ERROR "Cannot find macos sdk.")
                endif()
            endif()
        endif()
        message(STATUS "Setting CMAKE_OSX_SYSROOT for macos ${OSX_SHORT_VERSION} to ${XCODE_ISYSROOT}")
        set(CMAKE_OSX_SYSROOT ${XCODE_ISYSROOT})
    endif()
endif()

project (gfx LANGUAGES CXX VERSION 10.0.0)

# Tells if ngraph source tree is embedded in some other project
# TRUE if embedded in source tree
# FALSE if used standalone or as an external project
if(NOT ("${CMAKE_SOURCE_DIR}" STREQUAL "${PROJECT_SOURCE_DIR}"))
    set(GFX_IS_EMBEDDED TRUE)
else()
    set(GFX_IS_EMBEDDED FALSE)
endif()

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed.")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/local" CACHE PATH "..." FORCE)
endif()

set(GFX_CXX_STANDARD 17)
if(CMAKE_CXX_STANDARD)
    if(CMAKE_CXX_STANDARD VERSIONLESS GFX_CXX_STANDARD)
        message(FATAL_ERROR "CMAKE_CXX_STANDARD ${CMAKE_CXX_STANDARD} is lower than ${GFX_CXX_STANDARD}.")
    endif()
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create compilation database compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Setup CMAKE_ARGS to be forwarded to External Projects
set(GFX_FORWARD_CMAKE_ARGS
    -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
    -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
    -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
    -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=${CMAKE_EXPORT_COMPILE_COMMANDS}
    -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=${CMAKE_POSITION_INDEPENDENT_CODE}
    )

if (CMAKE_OSX_SYSROOT)
    set(GFX_FORWARD_CMAKE_ARGS
            ${NGRAPH_FORWARD_CMAKE_ARGS}
            -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
            )
endif()

if (NOT GFX_GENERATOR_IS_MULTI_CONFIG)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    endif()

    set(GFX_FORWARD_CMAKE_ARGS
            ${GFX_FORWARD_CMAKE_ARGS}
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            )
endif()
message(STATUS "GFX_FORWARD_CMAKE_ARGS ${GFX_FORWARD_CMAKE_ARGS}")

#-----------------------------------------------------------------------------------------------
# Installation logic...
#-----------------------------------------------------------------------------------------------

if (LINUX)
    include(GNUInstallDirs)
else()
    set(CMAKE_INSTALL_BINDIR "bin")
    set(CMAKE_INSTALL_INCLUDEDIR "include")
    set(CMAKE_INSTALL_DOCDIR "doc")
    set(CMAKE_INSTALL_LIBDIR "lib")
endif()

message(STATUS "Installation directory: ${CMAKE_INSTALL_PREFIX}")

# Destinations
if (LINUX)
    if (DEFINED GFX_RPATH)
        set(CMAKE_BUILD_RPATH "$ORIGIN:${NGRAPH_RPATH}")
        set(CMAKE_INSTALL_RPATH "$ORIGIN:${NGRAPH_RPATH}")
    else()
        set(CMAKE_BUILD_RPATH "$ORIGIN")
        set(CMAKE_INSTALL_RPATH "$ORIGIN")
    endif()
endif()

#-----------------------------------------------------------------------------------------------
# External projects install directory
#-----------------------------------------------------------------------------------------------

if (NOT GFX_BUILD_BASE_DIR)
    set(GFX_BUILD_BASE_DIR ${PROJECT_BINARY_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GFX_BUILD_BASE_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${NGRAPH_BUILD_DIR})
    set(CMAKE_PDB_OUTPUT_DIRECTORY ${NGRAPH_BUILD_DIR})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${NGRAPH_BUILD_DIR})
endif()

#-----------------------------------------------------------------------------------------------
# Compile Flags for xGraph...
#-----------------------------------------------------------------------------------------------

# Since UNIX support Bash we can use a Bash script to do the clang-format functions
# This is much faster than the cmake method
set(FORMAT_DIRS
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/test
)
if (UNIX)
    add_custom_target(style-check
        COMMAND
        ${NGRAPH_BASE_DIR}/maint/check-code-format.sh ${FORMAT_DIRS}
    )
    add_custom_target(style-apply
        COMMAND
        ${NGRAPH_BASE_DIR}/maint/apply-code-format.sh ${FORMAT_DIRS}
    )
    add_custom_target(style
        COMMAND
        ${NGRAPH_BASE_DIR}/maint/apply-code-format.sh ${FORMAT_DIRS}
    )
else()
    add_custom_target(style-check
        COMMAND ${CMAKE_COMMAND}
        -DNGRAPH_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        -P ${NGRAPH_BASE_DIR}/cmake/Modules/style_check.cmake
    )

    add_custom_target(style-apply
        COMMAND ${CMAKE_COMMAND}
        -DNGRAPH_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        -P ${NGRAPH_BASE_DIR}/cmake/Modules/style_apply.cmake
    )

    add_custom_target(style
        COMMAND ${CMAKE_COMMAND}
        -DNGRAPH_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
        -P ${NGRAPH_BASE_DIR}/cmake/Modules/style_apply.cmake
    )
endif()

add_custom_target(fix-mode
    COMMAND ${CMAKE_COMMAND}
    -DNGRAPH_SOURCE_DIR="${PROJECT_SOURCE_DIR}"
    -P ${NGRAPH_BBSE_DIR}/cmake/Modules/fix_mode.cmake
)

#-----------------------------------------------------------------------------------------------
# enable or disable output from GFX_DEBUG statements
#-----------------------------------------------------------------------------------------------
if(GFX_DEBUG_ENABLE)
    add_definitions(-DGFX_DEBUG_ENABLE)
endif()

#-----------------------------------------------------------------------------------------------
# enable or disable deprecation warnings for old APIs
#-----------------------------------------------------------------------------------------------
if(GFX_DEPRECATED_ENABLE)
    add_definitions(-DGFX_DEPRECATED_ENABLE)
endif()

add_definitions(-DPROJECT_ROOT_DIR="${PROJECT_SOURCE_DIR}")

#-----------------------------------------------------------------------------------------------
# Print Global Options
#-----------------------------------------------------------------------------------------------
get_directory_property(GFX_COMPILE_OPTIONS COMPILE_OPTIONS)
message(STATUS "Compile Flags: ${NGRAPH_COMPILE_OPTIONS}")
message(STATUS "Shared Link Flags: ${CMAKE_SHARED_LINKER_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_CXX_FLAGS_MINSIZEREL ${CMAKE_CXX_FLAGS_MINSIZEREL}")

#-----------------------------------------------------------------------------------------------
# xGraph source tree
#-----------------------------------------------------------------------------------------------

add_subdirectory(src)

add_subdirectory(test)

#-----------------------------------------------------------------------------------------------
# Export Config and Install
#-----------------------------------------------------------------------------------------------

# Not supported for now.
return()

include(CMakePackageConfigHelpers)

export(TARGETS gfx NAMESPACE gfx:: FILE "${PROJECT_BINARY_DIR}/gfxTargets.cmake")

install(EXPORT gfxTargets
        FILE gfxTargets.cmake
        NAMESPACE gfx::
        DESTINATION ${NGRAPH_COMPONENT_PREFIX}cmake
        COMPONENT gfx)

configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/share/gfxConfig.cmake.in
        ${PROJECT_BINARY_DIR}/ngraphConfig.cmake
        INSTALL_DESTINATION cmake)

write_basic_package_version_file(${PROJECT_BINARY_DIR}/gfxConfigVersion.cmake
        VERSION ${GFX_VERSION}
        COMPATIBILITY SameMajorVersion)

install(FILES ${PROJECT_BINARY_DIR}/ngraphConfig.cmake
        ${PROJECT_BINARY_DIR}/ngraphConfigVersion.cmake
        DESTINATION ${NGRAPH_COMPONENT_PREFIX}cmake
        COMPONENT gfx)

install(DIRECTORY
    ${PROJECT_SOURCE_DIR}/licenses
    DESTINATION "${NGRAPH_COMPONENT_PREFIX}."
    COMPONENT gfx
)

install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION "${GFX_COMPONENT_PREFIX}." COMPONENT gfx)
install(FILES ${PROJECT_BINARY_DIR}/VERSION DESTINATION "${GFX_COMPONENT_PREFIX}." COMPONENT gfx)
