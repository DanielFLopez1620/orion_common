cmake_minimum_required(VERSION 3.8)
project(orion_bringup)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(orion_description REQUIRED)
find_package(orion_control REQUIRED)
# find_package(orion_chat REQUIRED)
find_package(depth_maixsense_a010 REQUIRED)
find_package(depth_ydlidar_os30a REQUIRED)
# find_package(orbbec_camera REQUIRED)
# find_package(orbbec_camera_msgs REQUIRED)

install(
    DIRECTORY launch config
    DESTINATION share/${PROJECT\_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
