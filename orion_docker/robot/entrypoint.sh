#!/bin/bash
# Docker entrypoint for orion_robot.
# Sources ROS 2 and the built workspace before running the given command.
set -e

source /opt/ros/${ROS_DISTRO}/setup.bash
source /opt/microros_ws/install/setup.bash
source /home/orion_user/ws/install/setup.bash

# G Mov picam — source the host-mounted camera_ros install if present.
# Uses local_setup.bash (overlay only) to avoid sourcing host underlay paths
# that don't exist inside the container (e.g. /home/ubuntu/ros2_ws).
if [ -f /home/orion_user/picam_ws/install/local_setup.bash ]; then
    source /home/orion_user/picam_ws/install/local_setup.bash
fi

exec "$@"
