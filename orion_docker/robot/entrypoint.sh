#!/bin/bash
# Docker entrypoint for orion_robot.
# Sources ROS 2 and the built workspace before running the given command.
set -e

# Start pigpio daemon for G Mov servo GPIO control (requires --privileged).
if ! pgrep -x pigpiod > /dev/null; then
    pigpiod
    sleep 0.3
fi

source /opt/ros/${ROS_DISTRO}/setup.bash
source /opt/microros_ws/install/setup.bash
source /home/orion_user/ws/install/setup.bash

exec "$@"
