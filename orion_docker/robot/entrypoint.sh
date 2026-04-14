#!/bin/bash
# Docker entrypoint for orion_robot.
# Sources ROS 2 and the built workspace before running the given command.
set -e

source /opt/ros/${ROS_DISTRO}/setup.bash
source /home/orion_user/ws/install/setup.bash

exec "$@"
