#!/bin/bash
set -e

G_MOV="true"        # Options: true, false
CAMERA_TYPE="a010"  # Options: a010, astra_s, os30a
echo "Selected camera: $CAMERA_TYPE"

echo "Waiting for internet connection..."
while ! ping -c 1 8.8.8.8 &>/dev/null; do sleep 1; done
echo "Internet connection established."

for DEV in /dev/ttyESP32_1 /dev/ttyESP32_2 /dev/ttyLD19; do
    echo "Waiting for $DEV..."
    while [ ! -e "$DEV" ]; do sleep 1; done
    echo "$DEV found."
done

if [ "$CAMERA_TYPE" == "a010" ]; then
    while [ ! -e /dev/ttyA010 ]; do sleep 1; done
    echo "/dev/ttyA010 is available."
elif [ "$CAMERA_TYPE" == "astra_s" ]; then
    while ! lsusb | grep -i "astra_s" &>/dev/null; do sleep 1; done
    echo "Astra S USB device found."
fi

echo "Sourcing ROS2 and workspace..."
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash

echo "Launching ORION bringup..."
ros2 launch orion_bringup bringup.launch.py camera:=$CAMERA_TYPE g_mov:=$G_MOV