#!/bin/bash
# Docker entrypoint for orion_robot.
# Sources ROS 2 and the built workspace before running the given command.
set -e

source /opt/ros/${ROS_DISTRO}/setup.bash
source /opt/microros_ws/install/setup.bash
source /home/orion_user/ws/install/setup.bash

# G Mov picam — source the host-mounted camera_ros install if present.
# Bind-mounted by orion_robot.service when libcamera+camera_ros live on host.
if [ -f /home/orion_user/picam_ws/install/setup.bash ]; then
    source /home/orion_user/picam_ws/install/setup.bash
fi

exec "$@"
