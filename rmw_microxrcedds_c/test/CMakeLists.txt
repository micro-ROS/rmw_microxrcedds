find_package(ament_cmake_gtest REQUIRED)
find_package(ament_cmake_pytest REQUIRED)

find_package(osrf_testing_tools_cpp REQUIRED)

get_target_property(memory_tools_ld_preload_env_var
  osrf_testing_tools_cpp::memory_tools LIBRARY_PRELOAD_ENVIRONMENT_VARIABLE)


# finding gtest once in the highest scope
# prevents finding it repeatedly in each local scope
ament_find_gtest()


ament_add_gtest(test_node test_node.cpp)
if(TARGET test_node)
  target_link_libraries(test_node ${PROJECT_NAME})
endif()


#ament_add_gtest(test_node test_node.cpp)
#if(TARGET test_node)
#  target_link_libraries(test_node ${PROJECT_NAME})
#endif()


#ament_add_gtest(test_publisher test_publisher.cpp)
#if(TARGET test_publisher)
#  target_link_libraries(test_publisher ${PROJECT_NAME})
#endif()

#ament_add_gtest(test_subscription test_subscription.cpp)
#if(TARGET test_subscription)
#  target_link_libraries(test_subscription ${PROJECT_NAME})
#endif()


#if(NOT CMAKE_CXX_COMPILE_OBJECT)
#  set(CMAKE_CXX_COMPILE_OBJECT "<CMAKE_CXX_COMPILER>  <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
#endif()
#if(NOT CMAKE_CXX_LINK_EXECUTABLE)
#  set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER>  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>")
#endif()