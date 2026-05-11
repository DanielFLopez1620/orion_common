#!/usr/bin/env python3
"""Launch the ORION model visualization with RViz and joint_state_publisher_gui."""

# ///////////////////////////// REQUIRED LIBRARIES //////////////////////////////
# .............................. Python libraries ...............................
import os

# ............................ Launch dependencies .............................
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.actions import Node

# //////////////////////////// GLOBAL DEFINITIONS //////////////////////////////
ARGS = [
    DeclareLaunchArgument('camera', default_value='os30a',
        description="Choose a cam for the robot (os30a, astra_s, a010)",
        choices=['os30a', 'astra_s', 'a010']),
    DeclareLaunchArgument('servo', default_value='true',
        description="Boolean to include or not the servos",
        choices=['true', 'false']),
    DeclareLaunchArgument('g_mov', default_value='false',
        description="When using camera a010, whether to include or not G Mov",
        choices=['true', 'false']),
    DeclareLaunchArgument('rasp', default_value='rpi5',
        description="Select 4 for Raspberry Pi 4B, or 5 for Raspberry Pi 5",
        choices=['rpi4', 'rpi5']),
    DeclareLaunchArgument('ros2_control', default_value='false',
        description="Whether to use ros2_control tags for motor controllers",
        choices=['true', 'false']),
    DeclareLaunchArgument('simplified', default_value='false',
        description="To ignore non-functional components in the URDF description",
        choices=['true', 'false']),
    DeclareLaunchArgument('motor', default_value='100',
        description="Select your motor nominal speed (rpm) at 12V",
        choices=['1000', '100']),
    DeclareLaunchArgument('color', default_value='gold',
        description="Visual color for structural panels",
        choices=['gold', 'mdf', 'transparent', 'pla']),
]

# //////////////////////////// LAUNCH DEFINITION //////////////////////////////


def generate_launch_description():
    """Return the LaunchDescription for ORION model visualization."""
    ld = LaunchDescription(ARGS)

    pkg_description = get_package_share_directory('orion_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'orion.urdf.xacro')
    rviz_config_file = os.path.join(pkg_description, 'rviz', 'model_viz.rviz')

    ld.add_action(
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'robot_description': Command([
                    'xacro ', xacro_file,
                    ' camera:=', LaunchConfiguration('camera'),
                    ' servo:=', LaunchConfiguration('servo'),
                    ' g_mov:=', LaunchConfiguration('g_mov'),
                    ' rasp:=', LaunchConfiguration('rasp'),
                    ' gazebo:=false',
                    ' ros2_control:=', LaunchConfiguration('ros2_control'),
                    ' simplified:=', LaunchConfiguration('simplified'),
                    ' motor:=', LaunchConfiguration('motor'),
                    ' color:=', LaunchConfiguration('color'),
                    ' ctl_type:=micro_ros'
                ])
            }]
        )
    )

    ld.add_action(
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher_gui'
        )
    )

    ld.add_action(
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d', rviz_config_file],
            output='screen'
        )
    )

    return ld
