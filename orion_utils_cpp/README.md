# 🤖 ORION Utils C++

## 🌟 Overview

C++ mirror of `orion_utils_py`. Contains standalone ROS 2 nodes for ORION demos,
diagnostics, and testing — each executable maps 1-to-1 with its Python counterpart.

---

## 📝 License

The source code is released under a [BSD 3-Clause license](/LICENSE).

**Author**: Daniel Felipe López Escobar.

The *orion_utils_cpp* package has been tested under [ROS](https://www.ros.org/) Jazzy.

---

## 📚 Table of Contents

- [💻 Nodes](#-nodes)

---

## 💻 Nodes

### check_mov

Node that subscribes to */mobile_base_controller/cmd_vel* and */mobile_base_controller/odom* topics to measure the latency between sending a velocity command and detecting the first observed movement:

~~~bash
ros2 run orion_utils_cpp check_mov
~~~

### demo_theater

A demo node focused on executing a scripted performance sequence: forward motion, right turn, response message, emotions, and a final spin:

~~~bash
ros2 run orion_utils_cpp demo_theater
~~~

### emotion_try

A node that cycles through all ORION emotion indices (0–7) with synchronized arm sine-wave motion to check all the robot's expressions:

~~~bash
ros2 run orion_utils_cpp emotion_try
~~~

### happy_birthday

A simple node intended to greet a special person: publishes a TTS birthday message, sets a happy emotion, and oscillates both arms:

~~~bash
ros2 run orion_utils_cpp happy_birthday
~~~

### introducing_orion

A demo node that covers simple arm gestures, forward motion, and emotions to display the capabilities of ORION:

~~~bash
ros2 run orion_utils_cpp introducing_orion
~~~

### hi_human

A demo node that executes a greeting sequence: sets emotions, moves forward, waves the left arm, and rotates right:

~~~bash
ros2 run orion_utils_cpp hi_human
~~~

### laser_filter

Node oriented to filter the scan ranges in the four corners of the robot based on the LIDAR position:

~~~bash
ros2 run orion_utils_cpp laser_filter
~~~

### touch_interaction

C++ equivalent of `orion_utils_py`'s `touch_interaction`. Reacts to the four capacitive touch sensors with the same emotion/arm/TTS behaviors, using `std::atomic<bool>` for the busy flag and detached `std::thread` for non-blocking action sequences.

~~~bash
ros2 run orion_utils_cpp touch_interaction
~~~

> The Python variant (`orion_utils_py`) is the one launched by `orion_bringup`. This node is the C++ mirror for development or testing contexts.
