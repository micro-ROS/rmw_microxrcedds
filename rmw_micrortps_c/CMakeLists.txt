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

project(rmw_micrortps
    VERSION 0.1.0
    LANGUAGES C
    )

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)

find_package(micrortps_cmake_module REQUIRED)
find_package(microcdr REQUIRED CONFIG)
find_package(micrortps_client REQUIRED CONFIG)
find_package(MicroRTPS REQUIRED MODULE)

find_package(rmw REQUIRED)
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_typesupport_micrortps_c REQUIRED)
    
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME}
    microcdr
    micrortps_client
    )
    
ament_target_dependencies(${PROJECT_NAME}
    "rcutils"
    "rosidl_typesupport_micrortps_c"
    "rmw"
    "rosidl_generator_c"
)

configure_rmw_library(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)
set_target_properties(${PROJECT_NAME} PROPERTIES
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
    rosidl_typesupport_micrortps_c
    rosidl_generator_c
    rcutils
    rmw
    )

register_rmw_implementation(
    "c:rosidl_typesupport_c:rosidl_typesupport_micrortps_c"
    "cpp:rosidl_typesupport_cpp:rosidl_typesupport_introspection_cpp"
)

ament_package(
    CONFIG_EXTRAS_POST "rmw_micrortps-extras.cmake"
    )

install(
    DIRECTORY include/
    DESTINATION include
    )

install(
    TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    )
