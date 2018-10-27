# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake_gtest REQUIRED)
find_package(microxrcedds_agent_cmake_module REQUIRED)
find_package(MicroXRCEDDS_Agent REQUIRED MODULE)


set(TEST_UTILS_FILES_SOURCES "test_utils.cpp")
set(TEST_UTILS_FILES_HEADERS "test_utils.hpp")

# Node tests
set(TEST_NAME "test_node")
set(TEST_FILES "test_node.cpp")
ament_add_gtest(
  ${TEST_NAME}
  ${TEST_FILES}
  ${SRC_FILES}
  ${TEST_UTILS_FILES_SOURCES}
  TIMEOUT 5
)
if(TARGET ${TEST_NAME})
  ament_target_dependencies(
    ${TEST_NAME}
    ${PROJECT_NAME}
    microxrcedds_agent
    rmw
    rosidl_typesupport_microxrcedds_shared
  )

  target_link_libraries(
    ${TEST_NAME}
    microxrcedds_agent
    microxrcedds_client
    microcdr
  )

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()


# Publisher tests
set(TEST_NAME "test_publisher")
set(TEST_FILES "test_publisher.cpp")
ament_add_gtest(
  ${TEST_NAME}
  ${TEST_FILES}
  ${SRC_FILES}
  ${TEST_UTILS_FILES_SOURCES}
  TIMEOUT 5
)
if(TARGET ${TEST_NAME})
  ament_target_dependencies(
    ${TEST_NAME}
    ${PROJECT_NAME}
    microxrcedds_agent
    rmw
    rosidl_typesupport_microxrcedds_shared
  )

  target_link_libraries(
    ${TEST_NAME}
    microxrcedds_agent
    microxrcedds_client
    microcdr
  )

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()


# Subscriber tests
set(TEST_NAME "test_subscription")
set(TEST_FILES "test_subscription.cpp")
ament_add_gtest(
  ${TEST_NAME}
  ${TEST_FILES}
  ${SRC_FILES}
  ${TEST_UTILS_FILES_SOURCES}
  TIMEOUT 5
)
if(TARGET ${TEST_NAME})
  ament_target_dependencies(
    ${TEST_NAME}
    ${PROJECT_NAME}
    microxrcedds_agent
    rmw
    rosidl_typesupport_microxrcedds_shared
  )

  target_link_libraries(
    ${TEST_NAME}
    microxrcedds_agent
    microxrcedds_client
    microcdr
  )

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()


# Pubish and subcribe
set(TEST_NAME "test_pubsub")
set(TEST_FILES "test_pubsub.cpp")
ament_add_gtest(
  ${TEST_NAME}
  ${TEST_FILES}
  ${SRC_FILES}
  ${TEST_UTILS_FILES_SOURCES}
  TIMEOUT 5
)
if(TARGET ${TEST_NAME})
  ament_target_dependencies(
    ${TEST_NAME}
    ${PROJECT_NAME}
    microxrcedds_agent
    rmw
    rosidl_typesupport_microxrcedds_shared
  )

  target_link_libraries(
    ${TEST_NAME}
    microxrcedds_agent
    microxrcedds_client
    microcdr
  )

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()


# topic
set(TEST_NAME "test_topic")
set(TEST_FILES "test_topic.cpp")
ament_add_gtest(
  ${TEST_NAME}
  ${TEST_FILES}
  ${SRC_FILES}
  ${TEST_UTILS_FILES_SOURCES}
  TIMEOUT 5
)
if(TARGET ${TEST_NAME})
  ament_target_dependencies(
    ${TEST_NAME}
    ${PROJECT_NAME}
    microxrcedds_agent
    rmw
    rosidl_typesupport_microxrcedds_shared
  )

  target_link_libraries(
    ${TEST_NAME}
    microxrcedds_agent
    microxrcedds_client
    microcdr
  )

  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()
