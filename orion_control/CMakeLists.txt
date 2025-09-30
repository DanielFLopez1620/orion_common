cmake_minimum_required(VERSION 3.8)
project(orion_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(hardware_interface REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)

add_subdirectory(src)

pluginlib_export_plugin_description_file(hardware_interface ctl_orion.xml)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
