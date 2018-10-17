# Default to C++14
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

find_package(ament_cmake_gtest REQUIRED)
find_package(MicroXRCEDDS_Agent REQUIRED MODULE)



set(TEST_NAME "test_node")
set(TEST_FILES "test_node.cpp")
ament_add_gtest(
  ${TEST_NAME} 
  ${TEST_FILES}
  )
if(TARGET ${TEST_NAME})
  target_link_libraries(${TEST_NAME} ${PROJECT_NAME})
  target_link_libraries(${TEST_NAME} microxrcedds_agent)
  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()



set(TEST_NAME "test_publisher")
set(TEST_FILES "test_publisher.cpp")
ament_add_gtest(
  ${TEST_NAME} 
  ${TEST_FILES}
  )
if(TARGET ${TEST_NAME})
  target_link_libraries(${TEST_NAME} ${PROJECT_NAME})
  target_link_libraries(${TEST_NAME} microxrcedds_agent)
  target_include_directories(
    ${TEST_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/config>
  )
endif()