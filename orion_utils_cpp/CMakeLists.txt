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
find_package(nav_msgs REQUIRED)

add_executable(laser_filter src/laser_filter.cpp)
ament_target_dependencies(laser_filter rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(emotion_try src/emotion_try.cpp)
ament_target_dependencies(emotion_try rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(happy_birthday src/happy_birthday.cpp)
ament_target_dependencies(happy_birthday rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(check_mov src/check_mov.cpp)
ament_target_dependencies(check_mov rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(demo_theater src/demo_theater.cpp)
ament_target_dependencies(demo_theater rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(introducing_orion src/introducing_orion.cpp)
ament_target_dependencies(introducing_orion rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

add_executable(hi_human src/hi_human.cpp)
ament_target_dependencies(hi_human rclcpp std_msgs geometry_msgs sensor_msgs nav_msgs)

install(TARGETS
    laser_filter
    emotion_try
    happy_birthday
    check_mov
    demo_theater
    introducing_orion
    hi_human
    DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
    find_package(ament_lint_auto REQUIRED)
    set(ament_cmake_copyright_FOUND TRUE)
    set(ament_cmake_cpplint_FOUND TRUE)
    ament_lint_auto_find_test_dependencies()
endif()

ament_package()
