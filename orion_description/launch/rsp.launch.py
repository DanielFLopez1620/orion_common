# ///////////////////////////// REQUIRED LIBRARIES //////////////////////////////
# .............................. Python libraries ...............................
import os
import xacro

# ............................ Launch dependencies .............................
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

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
    DeclareLaunchArgument('ros2_control', default_value='false',
        description="Whether to use ros2_control tags for motor controllers",
        choices=['true', 'false']),
    DeclareLaunchArgument('simplified', default_value='false',
        description="To ignore no-functional components in the URDF description",
        choices=['true', 'false']),
    DeclareLaunchArgument('motor', default_value='100',
        description="Select your  motor nominal speed (rpm) at 12V",
        choices=['1000', '100']),
    DeclareLaunchArgument('ctl_type', default_value='micro_ros',
        description="Select controller communication option",
        choices=['serial', 'micro_ros']),
    DeclareLaunchArgument('gazebo', default_value='false',
        description="To use GZ configurations",
        choices=['true', 'false'])
]

# /////////////////////////// FUNCTIONS DEFINITIONS ////////////////////////////
def get_argument(context, arg):
    """
    Get the context when performing the Launch Configuration
    """
    return LaunchConfiguration(arg).perform(context)

def generate_robot_description(context):
    """
    For generating the robot description, consider the URDF/Xacro file provided,
    but modifying the meshes source path in order to make it available to
    Gazebo Harmonic.
    """
    # Paths to consider
    pkg_gmov = get_package_share_directory('g_mov_description')
    pkg_description = get_package_share_directory('orion_description')
    xacro_file = os.path.join(pkg_description, 'urdf', 'orion.urdf.xacro')

    # Generating mapping in order to allow xacro modularity
    mappings = {
        'camera': get_argument(context, "camera"),
        'servo': get_argument(context, "servo"),
        'g_mov': get_argument(context, "g_mov"),
        'rasp': get_argument(context, "rasp"),
        'gazebo': get_argument(context, "gazebo"),
        'ros2_control': get_argument(context, "ros2_control"),
        'simplified': get_argument(context, 'simplified'),
        'motor': get_argument(context, 'motor'),
        'ctl_type':get_argument(context, 'ctl_type')
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

    # Return configuration as a set
    return [rsp_node]

# /////////////////////////// LAUNCH DEFINITIONS //////////////////////////////
def generate_launch_description():
    # Generate launch description
    ld = LaunchDescription(ARGS)

    # Add robot description with context
    ld.add_action(OpaqueFunction(function=generate_robot_description))

    return ld
