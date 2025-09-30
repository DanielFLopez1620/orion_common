cmake_minimum_required(VERSION 3.8)
project(orion_utils_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(orion_control REQUIRED)

add_executable(orion_bridge_diff src/orion_bridge_diff.cpp)
ament_target_dependencies(orion_bridge_diff rclcpp std_msgs orion_control)
target_link_libraries(orion_bridge_diff orion_control)

add_executable(orion_bridge_forw src/orion_bridge_forw.cpp)
ament_target_dependencies(orion_bridge_forw rclcpp std_msgs orion_control)
target_link_libraries(orion_bridge_forw orion_control)

install(TARGETS
  orion_bridge_diff
  orion_bridge_forw
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
