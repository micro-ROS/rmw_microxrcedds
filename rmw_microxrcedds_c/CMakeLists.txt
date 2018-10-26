# Copyright 2018 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=*)
cmake_policy(SET CMP0057 NEW)


find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)

find_package(microxrcedds_cmake_module REQUIRED)
find_package(MicroXRCEDDS REQUIRED MODULE)

find_package(rmw REQUIRED)
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_generator_cpp REQUIRED)
#find_package(rosidl_typesupport_microxrcedds_c QUIET)
#find_package(rosidl_typesupport_microxrcedds_cpp QUIET)
find_package(rosidl_typesupport_microxrcedds_shared REQUIRED)

set(PACKAGE_VERSION "0.1.0")
if(BUILD_TESTING)
    project(rmw_microxrcedds
            VERSION ${PACKAGE_VERSION}
    )
else()
    project(rmw_microxrcedds
            VERSION ${PACKAGE_VERSION}
            LANGUAGES C
    )
endif()


if(EXISTS ${PROJECT_SOURCE_DIR}/rmw_microxrcedds.config)
    configure_file(${PROJECT_SOURCE_DIR}/rmw_microxrcedds.config ${PROJECT_SOURCE_DIR}/CMakeCache.txt COPYONLY)
    load_cache(${PROJECT_SOURCE_DIR})
    file(REMOVE ${PROJECT_SOURCE_DIR}/CMakeCache.txt)
else()
    message(FATAL_ERROR "No rmw_microxrcedds.config found.")
endif()

# Transport handle define macros.
set(MICRO_XRCEDDS_SERIAL OFF)
set(MICRO_XRCEDDS_UDP OFF)
if(${CONFIG_MICRO_XRCEDDS_TRANSPORT} STREQUAL "serial")
    set(MICRO_XRCEDDS_SERIAL ON)
elseif(${CONFIG_MICRO_XRCEDDS_TRANSPORT} STREQUAL "udp")
    set(MICRO_XRCEDDS_UDP ON)
else()
    message(FATAL_ERROR "rmw_microxrcedds.config transport not supported. Use \"serial\" or \"udp\"")
endif()

# Create entities type define macros.
set(MICRO_XRCEDDS_USE_REFS OFF)
set(MICRO_XRCEDDS_USE_XML OFF)
if(${CONFIG_MICRO_XRCEDDS_CREATION_MODE} STREQUAL "refs")
    set(MICRO_XRCEDDS_USE_REFS ON)
elseif(${CONFIG_MICRO_XRCEDDS_CREATION_MODE} STREQUAL "xml")
    set(MICRO_XRCEDDS_USE_XML ON)
else()
    message(FATAL_ERROR "rmw_microxrcedds.config creation mode not supported. Use \"refs\" or \"xmls\"")
endif()

# Create source files with the define
configure_file( ${PROJECT_SOURCE_DIR}/src/config.h.in
                ${PROJECT_BINARY_DIR}/config/config.h
)


file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME}
            ${SRC_FILES}
)
target_link_libraries(${PROJECT_NAME}
                      microcdr
                      microxrcedds_client
)

ament_target_dependencies(
    ${PROJECT_NAME}
    "rcutils"
    #"rosidl_typesupport_microxrcedds_c"
    #"rosidl_typesupport_microxrcedds_cpp"
    "rosidl_typesupport_microxrcedds_shared"
    "rmw"
    "rosidl_generator_c"
    "rosidl_generator_cpp"
)

configure_rmw_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
)
set_target_properties(${PROJECT_NAME}
                      PROPERTIES
                      C_STANDARD 99
                      C_STANDARD_REQUIRED YES
)

if(MSVC OR MSVC_IDE)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /Wall
    )
elseif(CMAKE_COMPILER_IS_GNUCC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror-implicit-function-declaration
        $<$<C_COMPILER_ID:GNU>:$<$<VERSION_GREATER:$<C_COMPILER_VERSION>,8.1>:-Wcast-align=strict>>
        -Wvla
        # -Werror
        # -pedantic-errors
    )
endif()

ament_export_include_directories(${PROJECT_SOURCE_DIR}/include)
ament_export_libraries(${PROJECT_NAME})

ament_export_dependencies(
    rosidl_typesupport_microxrcedds_c
    rosidl_typesupport_microxrcedds_cpp
    rosidl_generator_c
    rosidl_generator_cpp
    rcutils
    rmw
)

register_rmw_implementation(
    "c:rosidl_typesupport_c:rosidl_typesupport_microxrcedds_c"
    "cpp:rosidl_typesupport_cpp:rosidl_typesupport_microxrcedds_cpp"
)

if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    ament_lint_auto_find_test_dependencies()

    add_subdirectory(test)
endif()

ament_package(CONFIG_EXTRAS_POST
"rmw_microxrcedds-extras.cmake"
)

install(DIRECTORY include/
DESTINATION include
)

install(TARGETS ${PROJECT_NAME}
ARCHIVE DESTINATION lib
LIBRARY DESTINATION lib
RUNTIME DESTINATION bin
)