# ///////////////////////////// REQUIRED LIBRARIES //////////////////////////////
# .............................. Python libraries ...............................
import os
import xacro

# ............................ Launch dependencies .............................
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (DeclareLaunchArgument, IncludeLaunchDescription,
    OpaqueFunction)
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
        description="Choose a cam for the robot (os30a, astra_s, a010)",
        choices=['os30a', 'astra_s', 'a010']),
    DeclareLaunchArgument('servo',default_value='true',
        description="Boolean to include or not the servos",
        choices=['true', 'false']),
    DeclareLaunchArgument('g_mov',default_value='false',
        description="When using camera a010, whether to include or not G Mov",
        choices=['true', 'false']),
    DeclareLaunchArgument('rasp', default_value='rpi5',
        description="Select 4 for Raspberry Pi 4B, or 5 for Raspberry Pi 5",
        choices=['rpi4', 'rpi5']),
    DeclareLaunchArgument('simplified', default_value='false',
        description="To ignore no-functional components in the URDF description",
        choices=['true', 'false']),
    DeclareLaunchArgument('ctl_type', default_value='micro_ros',
        description="Select controller communication option: micro_ros or serial",
        choices=['micro_ros', 'serial']),
    DeclareLaunchArgument('motor', default_value='100',
        description="Select your  motor nominal speed (rpm) at 12V",
        choices=['1000', '100']),
]

def get_argument(context, arg):
    """
    Get the context when performing the Launch Configuration
    """
    return LaunchConfiguration(arg).perform(context)

def load_controllers(context):
    """
    Load controllers by considering OpaqueFunctions to allow all of them to load
    in a asynchronous way.

    It is used as recommended in:
    https://github.com/pal-robotics/tiago_robot/blob/humble-devel/tiago_controller_configuration/launch/default_controllers.launch.py

    Params
    ---
    Context : context
        Context provided by OpaqueFunction

    Returns
    ---
    controller : Array of launch description actions
        Actions linked to the controllers spawners
    """
    pkg_ctl = get_package_share_directory('orion_control')
    mobile_base_path = os.path.join(pkg_ctl, 'config', 'mobile_base_controller.yaml')
    joint_broad_path = os.path.join(pkg_ctl, 'config', 'joint_state_broadcaster.yaml')
    left_arm_path = os.path.join(pkg_ctl, 'config', 'simple_left_arm_controller.yaml')
    right_arm_path = os.path.join(pkg_ctl, 'config', 'simple_right_arm_controller.yaml')


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

    return controllers

def generate_robot_bringup(context):
    """
    For generating the robot description, consider the URDF/Xacro file provided,
    but modifying the meshes source path in order to make it available to
    Gazebo Harmonic.
    """
    # Paths to consider
    pkg_gmov = get_package_share_directory('g_mov_description')
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

def setup_lidar(context):
    """
    The LD19 node uses lifecycle, so here it is the implementation of the
    container for a proper usage of the LIDAR
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

    # Lode LiDAR Nodes
    load_composable_node = LoadComposableNodes(
        target_container='ldlidar_container',
        composable_node_descriptions=[ldlidar_component]
    )
    lidar_elements.append(load_composable_node)

    return lidar_elements


def generate_launch_description():
    """
    Launch description for common bringup of the ORION robot
    """
    # Paths
    lidar_config = os.path.join(
        get_package_share_directory('orion_bringup'),
        'config', 'lidar_lifecycle_mgr.yaml'
    )

    orion_chat_launch_path = os.path.join(
        get_package_share_directory('orion_chat'),
        'launch',
        'orion_launch.py'
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
        ]
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
        ]
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

    # To enable IA/TTS/STT/LLM functionalities
    #ld.add_action(IncludeLaunchDescription(
    #    PythonLaunchDescriptionSource(orion_chat_launch_path)
    #))

    return ld
