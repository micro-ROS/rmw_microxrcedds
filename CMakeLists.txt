cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=*)
cmake_policy(SET CMP0057 NEW)

project(rmw_micrortps
    VERSION 0.1.0
    LANGUAGES C
    )


# include(CTest)
# enable_testing()
find_package(microcdr REQUIRED)
find_package(micrortps_client REQUIRED)

find_package(ament_cmake_ros REQUIRED)
find_package(rcutils REQUIRED)

find_package(rmw REQUIRED)
find_package(rosidl_generator_c REQUIRED)
find_package(rosidl_typesupport_micrortps_c REQUIRED)
   
    
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
add_library(${PROJECT_NAME} ${SRC_FILES})
    
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
target_compile_options(${PROJECT_NAME}
    PRIVATE
        -Wall
        -Wextra
        -pedantic
        -Werror-implicit-function-declaration
        -Wcast-align=strict
        # -Werror
        # -pedantic-errors
    )

ament_export_include_directories(${PROJECT_SOURCE_DIR}/include)
ament_export_libraries(${PROJECT_NAME})


target_link_libraries(${PROJECT_NAME} micrortps_client)
ament_export_dependencies(
    rosidl_typesupport_micrortps_c
    rosidl_generator_c
    rcutils
    rmw
    )

register_rmw_implementation(
    "c:rosidl_typesupport_c:rosidl_typesupport_micrortps_c"
    # "cpp:rosidl_typesupport_cpp:rosidl_typesupport_introspection_cpp"
)

ament_package()

install(
  TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# set(CPACK_PROJECT_NAME ${PROJECT_NAME})
# set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include(CPack)