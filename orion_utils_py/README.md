# 🤖 ORION Python Utilities Package

## 🌟 Overview

This package is oriented to common utilities made with **Python** and **rclpy** for the ORION robot.

---

## 📝 License

The source code is released under a [BSD 3-Clause license](/orion_utils_py/LICENSE.md).

**Authors**: Daniel Felipe López Escobar.
The ORION Commons packages have been tested under [ROS](https://www.ros.org/) **Jazzy** distribution.

---

## 📚 Table of Contents

- [💻 Nodes](#-nodes)
- [⚠️ Troubleshooting](#️-troubleshooting)

## 💻 Nodes

### check_mov

Node that subscribes to */cmd_vel* and */odom* topics to measure the latency between sending a velocity command and detecting the first observed movement:

~~~bash
ros2 run orion_utils_py check_mov
~~~

### demo_theater

A demo node focused on say the final words of a theater presentation:

~~~bash
ros2 run orion_utils_py demo_theater
~~~

### emotion_try

A node that publishes the emotion with a incremental counter to check all the ORION's expressions:

~~~bash
ros2 run orion_utils_py emotion_try
~~~

### happy_birthday

A simple node intended to say hi to that special person:

~~~bash
ros2 run orion_utils_py happy_birthday
~~~

### introducing_orion

A demo node that covers simple arm and motor moves with emotions to display the capabilities of ORION.

~~~bash
ros2 run orion_utils_py introducing_orion
~~~

### laser_filter

Node oriented to filter the four corners of the robot based on the position of the LIDAR.
You can run it with:

~~~bash
ros2 run orion_utils_py laser_filter
~~~

### touch_interaction

Node that reacts to capacitive touch sensor events from ORION's four sensors (`touch_ur`, `touch_ul`, `touch_lr`, `touch_ll`) by triggering emotion, arm, and TTS responses. Uses rising-edge detection so actions fire once per touch, not on hold. A busy flag prevents overlapping sequences.

| Sensor topic | Behavior | Emotion |
|---|---|---|
| `/interaction/touch_ur` | Tickle: both arms flail x2 | Happy (4) |
| `/interaction/touch_ul` | Greet: left arm wave x2 | Excited (1) |
| `/interaction/touch_lr` | Think: right arm rises slowly | Thoughtful (2) |
| `/interaction/touch_ll` | Sad: both arms droop | Sad (6) |

~~~bash
ros2 run orion_utils_py touch_interaction
~~~

> The node is launched automatically by `orion_bringup` when `servo:=true`.

## ⚠️ Troubleshooting

### Ranges do not filter the columns of the robot

In case the position of your LIDAR differs from the one mentioned in the Wiki or in the robot_description, you can manually changes the ranges as it is managed as a parameter.

In the case of a execution, you can use:

~~~bash
ros2 run your_package your_node \
  --ros-args -p filter_ranges:="[0.5, 1.0, 2.2, 2.6, 3.7, 4.1, 5.3, 5.7]"
~~~

In the case of a Python Launch, you can use:

~~~bash
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='your_package',
            executable='your_node',
            name='laser_filter',
            parameters=[{
                'filter_ranges': [0.5, 1.0, 2.2, 2.6, 3.7, 4.1, 5.3, 5.7]
            }]
        )
    ])
~~~
