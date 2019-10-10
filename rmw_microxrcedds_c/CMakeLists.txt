# Copyright 2019 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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
cmake_policy(SET CMP0057 NEW)

find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
  set(CMAKE_C_CLANG_TIDY clang-tidy -checks=* -extra-arg=-Wno-unknown-warning-option)
endif()

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)

find_package(microxrcedds_cmake_module REQUIRED)
find_package(MicroXRCEDDS REQUIRED MODULE)

find_package(rmw REQUIRED)
find_package(rosidl_typesupport_microxrcedds_c REQUIRED)
find_package(rosidl_typesupport_microxrcedds_cpp REQUIRED)

set(PACKAGE_VERSION "0.1.0")
project(rmw_microxrcedds
  VERSION
    ${PACKAGE_VERSION}
  LANGUAGES
    C
  )

if(EXISTS ${PROJECT_SOURCE_DIR}/rmw_microxrcedds.config)
  configure_file(${PROJECT_SOURCE_DIR}/rmw_microxrcedds.config
    ${PROJECT_SOURCE_DIR}/CMakeCache.txt COPYONLY
    )
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
configure_file(${PROJECT_SOURCE_DIR}/src/config.h.in
  ${PROJECT_BINARY_DIR}/include/rmw_microxrcedds_c/config.h
  )

set(SRCS
  src/identifiers.c
  src/memory.c
  src/rmw_client.c
  src/rmw_compare_gids_equal.c
  src/rmw_count.c
  src/rmw_get_gid_for_publisher.c
  src/rmw_get_implementation_identifier.c
  src/rmw_get_serialization_format.c
  src/rmw_guard_condition.c
  src/rmw_init.c
  src/rmw_logging.c
  src/rmw_microxrcedds_topic.c
  src/rmw_node.c
  src/rmw_node_info_and_types.c
  src/rmw_node_names.c
  src/rmw_publish.c
  src/rmw_publisher.c
  src/rmw_request.c
  src/rmw_response.c
  src/rmw_serialize.c
  src/rmw_service.c
  src/rmw_service_names_and_types.c
  src/rmw_service_server_is_available.c
  src/rmw_subscription.c
  src/rmw_take.c
  src/rmw_topic_names_and_types.c
  src/rmw_trigger_guard_condition.c
  src/rmw_wait.c
  src/rmw_wait_set.c
  src/types.c
  src/utils.c
  )

add_library(${PROJECT_NAME}
  ${SRCS}
  )

target_link_libraries(${PROJECT_NAME}
  microcdr
  microxrcedds_client
  )

if(NOT WIN32)
  ament_export_libraries(pthread)
endif()

ament_target_dependencies(${PROJECT_NAME}
  "rcutils"
  "rosidl_typesupport_microxrcedds_c"
  "rosidl_typesupport_microxrcedds_cpp"
  "rmw"
  )

configure_rmw_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  )

set_target_properties(${PROJECT_NAME} PROPERTIES
  C_STANDARD
    99
  C_STANDARD_REQUIRED
    YES
  )

target_compile_options(${PROJECT_NAME}
  PRIVATE
    $<$<C_COMPILER_ID:GNU>:-Wall>
    $<$<C_COMPILER_ID:GNU>:-Wextra>
    $<$<C_COMPILER_ID:GNU>:-pedantic>
    $<$<C_COMPILER_ID:GNU>:-Werror-implicit-function-declaration>
    $<$<C_COMPILER_ID:GNU>:$<$<VERSION_GREATER:$<C_COMPILER_VERSION>,8.1>:-Wcast-align=strict>>
    $<$<C_COMPILER_ID:GNU>:-Wvla>
    $<$<C_COMPILER_ID:GNU>:-Werror>
    $<$<C_COMPILER_ID:GNU>:-pedantic-errors>
    $<$<C_COMPILER_ID:MSVC>:/Wall>
  )

ament_export_include_directories(${PROJECT_SOURCE_DIR}/include)
ament_export_libraries(${PROJECT_NAME})

ament_export_dependencies(
  rcutils
  rmw
  )

if(rosidl_typesupport_microxrcedds_c_FOUND)
  ament_export_dependencies(rosidl_typesupport_microxrcedds_c)
  set(implementations
    "c:rosidl_typesupport_c:rosidl_typesupport_microxrcedds_c"
    )
endif()
if(rosidl_typesupport_microxrcedds_cpp_FOUND)
  ament_export_dependencies(rosidl_typesupport_microxrcedds_cpp)
  set(implementations
    ${implementations}
    "cpp:rosidl_typesupport_cpp:rosidl_typesupport_microxrcedds_cpp"
    )
endif()
register_rmw_implementation(${implementations})


if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()

    #    add_subdirectory(test)
endif()

ament_package(CONFIG_EXTRAS_POST
  "rmw_microxrcedds-extras.cmake"
  )

# Install config.h file.
install(
  FILES
    ${PROJECT_BINARY_DIR}/include/rmw_microxrcedds_c/config.h
  DESTINATION
    include/rmw_microxrcedds_c
  )

# Install includes.
install(
  DIRECTORY
    ${PROJECT_SOURCE_DIR}/include/
  DESTINATION
    include
  )

# Install library.
install(
  TARGETS
    ${PROJECT_NAME}
  ARCHIVE DESTINATION
    lib
  LIBRARY DESTINATION
    lib
  RUNTIME DESTINATION
    bin
  )
