"""
bringup.launch.py — ORION robot full bringup for real-hardware deployment.

Launches: robot_state_publisher, ros2_control_node, micro-ROS agents (ESP32_1
and ESP32_2), LD19 LIDAR (lifecycle), depth camera driver (A010 or OS30A),
laser filter, and ros2_controllers.

Launch arguments:
  camera        : 'os30a' | 'a010'  (default: 'os30a')
  servo         : 'true'  | 'false' (default: 'true')
  g_mov         : 'true'  | 'false' (default: 'false', A010 only)
  rasp          : 'rpi4'  | 'rpi5'  (default: 'rpi5')
  simplified    : 'true'  | 'false' (default: 'false')
  ctl_type      : 'micro_ros' | 'serial' (default: 'micro_ros')
  motor         : '100'   | '1000'  (default: '100', nominal RPM at 12V)
  calibrate_imu : 'true'  | 'false' (default: 'false', g_mov only)
                  Force IMU re-calibration even if the calibration file exists.
                  If the file is absent, calibration always runs first automatically.
"""

# ///////////////////////////// REQUIRED LIBRARIES //////////////////////////////
# .............................. Python libraries ...............................
import os
import xacro

# ............................ Launch dependencies .............................
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, IncludeLaunchDescription,
    OpaqueFunction, RegisterEventHandler)
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import (LaunchConfiguration, Command, PathJoinSubstitution,
    PythonExpression)
from launch.conditions import IfCondition

from launch_ros.actions import (Node, ComposableNodeContainer,
    LoadComposableNodes)
from launch_ros.descriptions import ComposableNode
from launch_ros.substitutions import FindPackageShare
from launch_ros.parameter_descriptions import ParameterValue

# ........................... Additional packages dependencies ..................
from controller_manager.launch_utils import generate_load_controller_launch_description

# //////////////////////////// GLOBAL DEFINITIONS //////////////////////////////
ARGS = [
    DeclareLaunchArgument('camera', default_value='os30a',
        description="Choose a cam for the robot (os30a, a010). astra_s pending driver support.",
        choices=['os30a', 'a010']),
    DeclareLaunchArgument('servo', default_value='true',
        description="Boolean to include or not the servos",
        choices=['true', 'false']),
    DeclareLaunchArgument('g_mov', default_value='false',
        description="When using camera a010, whether to include or not G Mov",
        choices=['true', 'false']),
    DeclareLaunchArgument('rasp', default_value='rpi5',
        description="Select 4 for Raspberry Pi 4B, or 5 for Raspberry Pi 5",
        choices=['rpi4', 'rpi5']),
    DeclareLaunchArgument('simplified', default_value='false',
        description="To ignore non-functional components in the URDF description",
        choices=['true', 'false']),
    DeclareLaunchArgument('ctl_type', default_value='micro_ros',
        description="Select controller communication option: micro_ros or serial",
        choices=['micro_ros', 'serial']),
    DeclareLaunchArgument('motor', default_value='100',
        description="Select your motor nominal speed (rpm) at 12V",
        choices=['1000', '100']),
    DeclareLaunchArgument('calibrate_imu', default_value='false',
        description="(g_mov only) Force IMU re-calibration even if a calibration file "
                    "already exists. When the file is absent calibration always runs first.",
        choices=['true', 'false']),
]

def get_argument(context, arg):
    """
    Resolve a launch argument to its string value at execution time.

    Args:
        context: OpaqueFunction execution context.
        arg: Name of the declared launch argument.

    Returns:
        Resolved string value of the argument.
    """
    return LaunchConfiguration(arg).perform(context)

def load_controllers(context):
    """
    Spawn ros2_controllers via OpaqueFunction for asynchronous loading.

    Always loads mobile_base_controller and joint_state_broadcaster.
    Adds left/right arm controllers when the 'servo' argument is 'true'.
    Pattern recommended by PAL Robotics for controller spawning.

    Args:
        context: OpaqueFunction execution context.

    Returns:
        List of launch description actions for each controller spawner.
    """
    pkg_ctl = get_package_share_directory('orion_control')
    mobile_base_path = os.path.join(pkg_ctl, 'config', 'mobile_base_controller.yaml')
    joint_broad_path = os.path.join(pkg_ctl, 'config', 'joint_state_broadcaster.yaml')
    left_arm_path    = os.path.join(pkg_ctl, 'config', 'simple_left_arm_controller.yaml')
    right_arm_path   = os.path.join(pkg_ctl, 'config', 'simple_right_arm_controller.yaml')
    g_mov_path       = os.path.join(pkg_ctl, 'config', 'g_mov_servo_controller.yaml')


    controllers = [
        generate_load_controller_launch_description(
            controller_name="mobile_base_controller",
            controller_params_file=mobile_base_path)
    ]

    controllers.append(generate_load_controller_launch_description(
        controller_name="joint_state_broadcaster",
        controller_params_file=joint_broad_path
    ))

    if LaunchConfiguration('servo').perform(context) == 'true':
        controllers.append(generate_load_controller_launch_description(
            controller_name="simple_left_arm_controller",
            controller_params_file=left_arm_path
        ))
        controllers.append(generate_load_controller_launch_description(
            controller_name="simple_right_arm_controller",
            controller_params_file=right_arm_path
        ))

    if LaunchConfiguration('g_mov').perform(context) == 'true':
        controllers.append(generate_load_controller_launch_description(
            controller_name="g_mov_servo_controller",
            controller_params_file=g_mov_path
        ))

    return controllers

def generate_robot_bringup(context):
    """
    Build the robot description from xacro and return the core bringup nodes.

    Processes orion.urdf.xacro with the resolved launch arguments as mappings,
    then returns a robot_state_publisher node and a ros2_control_node.

    Args:
        context: OpaqueFunction execution context.

    Returns:
        List containing [rsp_node, controller_node].
    """
    pkg_description = get_package_share_directory('orion_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'orion.urdf.xacro')
    pkg_control = get_package_share_directory('orion_control')
    controller_params = os.path.join(pkg_control, 'config', 'control_manager.yaml')

    # Generating mapping in order to allow xacro modularity
    mappings = {
        'camera': get_argument(context, "camera"),
        'servo': get_argument(context, "servo"),
        'g_mov': get_argument(context, "g_mov"),
        'rasp': get_argument(context, "rasp"),
        'gazebo': 'false',
        'ros2_control': 'true',
        'simplified': get_argument(context, 'simplified'),
        'ctl_type': get_argument(context, 'ctl_type'),
        'motor': get_argument(context, 'motor')
    }

    # Obtaining robot description and making the substitution
    robot_description_config = xacro.process_file(xacro_file, mappings=mappings)
    robot_desc = robot_description_config.toprettyxml(indent='  ')

    # Launch node for robot state publisher
    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name="robot_state_publisher",
        output='screen',
        parameters=[{
            'robot_description': robot_desc,
            'rate': 200,
        }]
    )

    # Launch node for controller manager
    controller_node = Node(
        package="controller_manager",
        executable="ros2_control_node",
        parameters=[
                {'robot_description': robot_desc},
                controller_params
            ],
    )

    # Return configuration as a set
    return [rsp_node, controller_node]

_CAL_FILE = os.path.expanduser('~/.ros/mpu6050_calibration.yaml')


def setup_g_mov_imu(context):
    """
    Conditionally launch the MPU6050 IMU node, auto-calibrating when needed.

    Rules (only active when g_mov:=true):
      - Calibration file present  AND calibrate_imu:=false
          → launch mpu6050_node directly.
      - Calibration file absent   OR  calibrate_imu:=true
          → launch mpu6050_calibration first (non-interactive);
            when it exits, automatically launch mpu6050_node via OnProcessExit.

    Args:
        context: OpaqueFunction execution context.

    Returns:
        List of launch actions (Node and/or RegisterEventHandler).
    """
    if LaunchConfiguration('g_mov').perform(context) != 'true':
        return []

    imu_node = Node(
        package='orion_utils_py',
        executable='mpu6050_node',
        name='mpu6050_node',
        output='screen',
    )

    force_cal = LaunchConfiguration('calibrate_imu').perform(context) == 'true'
    needs_cal = force_cal or not os.path.isfile(_CAL_FILE)

    if not needs_cal:
        return [imu_node]

    cal_node = Node(
        package='orion_utils_py',
        executable='mpu6050_calibration',
        name='mpu6050_calibration',
        output='screen',
        arguments=['--yes'],
    )
    return [
        cal_node,
        RegisterEventHandler(
            OnProcessExit(target_action=cal_node, on_exit=[imu_node])
        ),
    ]


def setup_lidar(context):
    """
    Set up the LD19 LIDAR inside an isolated composable node container.

    The ldlidar_component runs as a lifecycle node managed by nav2_lifecycle_manager.
    Intra-process communication is enabled for lower latency.

    Args:
        context: OpaqueFunction execution context.

    Returns:
        List containing [ldlidar_container, load_composable_node].
    """
    lidar_elements = []
    lidar_params = os.path.join(get_package_share_directory('orion_bringup'),
        'config', 'ldlidar.yaml')

    # Add composable container for isolated components
    ldlidar_container = ComposableNodeContainer(
        name='ldlidar_container',
        package='rclcpp_components',
        namespace='',
        executable='component_container_isolated',
        composable_node_descriptions=[],
        output='screen',
    )
    lidar_elements.append(ldlidar_container)

    # Add composable node for the LIDAR component
    ldlidar_component = ComposableNode(
        package='ldlidar_component',
        plugin='ldlidar::LdLidarComponent',
        name='ldlidar_node',
        parameters=[lidar_params],
        extra_arguments=[{'use_intra_process_comms': True}]
    )

    # Load LiDAR node into the container
    load_composable_node = LoadComposableNodes(
        target_container='ldlidar_container',
        composable_node_descriptions=[ldlidar_component]
    )
    lidar_elements.append(load_composable_node)

    return lidar_elements


def generate_launch_description():
    """
    Assemble and return the full ORION bringup LaunchDescription.
    """
    # Paths
    lidar_config = os.path.join(
        get_package_share_directory('orion_bringup'),
        'config', 'lidar_lifecycle_mgr.yaml'
    )

    ld = LaunchDescription(ARGS)

    ld.add_action(Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        output="screen",
        parameters=[lidar_config]
    ))

    ld.add_action(Node(
        package='micro_ros_agent',
        executable="micro_ros_agent",
        name='micro_ros_agent_actuators',
        output="screen",
        arguments=[
            "serial",
            "--dev",
            "/dev/ttyESP32_1"
        ],
        condition=IfCondition(PythonExpression(["'", LaunchConfiguration('ctl_type'), "' == 'micro_ros'"]))
    ))

    ld.add_action(Node(
        package='micro_ros_agent',
        executable="micro_ros_agent",
        name='micro_ros_agent_interaction',
        output="screen",
        arguments=[
            "serial",
            "--dev",
            "/dev/ttyESP32_2"
        ],
        condition=IfCondition(PythonExpression(["'", LaunchConfiguration('ctl_type'), "' == 'micro_ros'"]))
    ))

    # Laser filter related to prevent considering self as obstacle
    ld.add_action(Node(
        package='orion_utils_py',
        executable='laser_filter',
        name='laser_filter',
        output='screen'
    ))

    # A010 driver run
    ld.add_action(Node(
        package='depth_maixsense_a010',
        executable='publisher',
        name='depth_maixsense_a010_publisher',
        parameters=[{'device': '/dev/ttyA010'}],
        condition=IfCondition(PythonExpression(["'", LaunchConfiguration('camera'), "' == 'a010'"]))
    ))

    # OS30A driver run
    os30a_launch_path = os.path.join(
        get_package_share_directory('depth_ydlidar_os30a'),
        'launch',
        'apc_camera_launch.py'
    )
    ld.add_action(IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os30a_launch_path),
        condition=IfCondition(PythonExpression(["'", LaunchConfiguration('camera'), "' == 'os30a'"]))
    ))

    # Astra S driver run
    # Make sure you can compile the Astra S packages on RPi before trying to use it
    #astra_launch_path = os.path.join(
    #    get_package_share_directory('orbbec_camera'),
    #    'launch',
    #    'astra.launch.py'
    #)

    #ld.add_action(IncludeLaunchDescription(
    #    PythonLaunchDescriptionSource(astra_launch_path),
    #    condition=IfCondition(PythonExpression(["'", LaunchConfiguration('camera'), "' == 'astra_s'"]))
    #))

    # Add execution of the description, controllers and LIDAR
    ld.add_action(OpaqueFunction(function=generate_robot_bringup))
    ld.add_action(OpaqueFunction(function=load_controllers))
    ld.add_action(OpaqueFunction(function=setup_lidar))

    # G Mov module nodes — only when g_mov:=true
    # IMU: OpaqueFunction handles calibrate_imu logic and file-existence check.
    ld.add_action(OpaqueFunction(function=setup_g_mov_imu))

    ld.add_action(Node(
        package='orion_utils_py',
        executable='g_mov_servo_node',
        name='g_mov_servo_node',
        output='screen',
        condition=IfCondition(
            PythonExpression(["'", LaunchConfiguration('g_mov'), "' == 'true'"])
        ),
    ))

    # G Mov picam — camera_ros bridge for the OV5647 (Pi Camera v1.3).
    # Built and installed on the host (see docs/cam/README.md); the container
    # mounts /usr/local + the host's picam_ws install at runtime.
    # FrameDurationLimits is in microseconds: [100000, 100000] locks ~10 fps.
    ld.add_action(Node(
        package='camera_ros',
        executable='camera_node',
        name='g_mov_picam',
        namespace='g_mov',
        output='screen',
        parameters=[{
            'format': 'YUYV',
            'width': 320,
            'height': 240,
            'FrameDurationLimits': [100000, 100000],
            'frame_id': 'g_mov_picam',
        }],
        condition=IfCondition(
            PythonExpression(["'", LaunchConfiguration('g_mov'), "' == 'true'"])
        ),
    ))

    return ld
