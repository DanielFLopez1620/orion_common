add_library(orion_control SHARED diffdrive_orion.cpp wheel.cpp forward_orion.cpp)

target_include_directories(orion_control PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/orion_control>
)

ament_target_dependencies(orion_control PUBLIC
    hardware_interface
    pluginlib
    rclcpp
    rclcpp_lifecycle
)

install(
    TARGETS orion_control
    EXPORT orion_control
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

ament_export_targets(orion_control HAS_LIBRARY_TARGET)
ament_export_dependencies(hardware_interface pluginlib rclcpp rclcpp_lifecycle)