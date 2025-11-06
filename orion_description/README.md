# 🤖 ORION Description

## 🌟 Overview

Package oriented to the deployment of the robot description by using URDF/Xacro files.

---

## 📝 License

The source code is released under a [BSD 3-Clause license](/LICENSE).

**Author**: Daniel Felipe López Escobar.

The *orion_description* package has been tested under [ROS](https://www.ros.org/) Jazzy.

---

## 📚 Table of Contents

- [🚀 Launch files](#-launch-files)
- [⚙️ RViz2 Configs](#️-rviz2-configs)
- [⚠️ Troubleshooting](#️-troubleshooting)

---

## 🚀 Launch files

Make sure you have followed the [installation process](/README.md) and have sourced your installation before following to the next steps:

### Model visualization

You can visualize the */robot_description* of the robot with the [model_vis.launch.py](/orion_description/launch/model_vis.launch.py), which provide a visualization window with RViz and the usage of the **robot_state_publisher_gui**.

~~~bash
# Basic usage:
# ros2 launch orion_description model_vis.launch.py
# Additional arguments:
#   camera : Can be 'astra_s', 'a010' or 'os30a'.
#   g_mov : Boolean (true/false) to use g_mov module when using 'a010' depth cam.
#   rasp : Whether to use 'rpi4' or 'rpi5', this will imply a change in the sound hardware.
#   servo : Boolean (true/false) to indicate if use servo arms
#   ros2_control : Boolean (true/false) to indicate usage of ros2_controllers
#   simplified : Boolean (true/false) to indicate if use the simplified URDF model
#
ros2 launch orion_description model_vis.launch.py camera:=os30a
~~~

![model_vis](https://github.com/Tesis-ORION/orion_common/blob/main/docs/readmes/model_viz.gif)

### Robot State Publisher

To launch the */robot_description* of the robot with the [rsp.launch.py](/orion_description/launch/rsp.launch.py), which will allow you to have the topic and the transforms of the model for simulations or raw visualizations.

~~~bash
# Basic usage:
# ros2 launch orion_description rsp.launch.py
# Additional arguments:
#   camera : Can be 'astra_s', 'a010' or 'os30a'.
#   gazebo : Boolean (true/false) to indicate the usage of GZ Sim.
#   g_mov : Boolean (true/false) to use g_mov module when using 'a010' depth cam.
#   rasp : Whether to use 'rpi4' or 'rpi5', this will imply a change in the sound hardware.
#   servo : Boolean (true/false) to indicate if use servo arms
#   simplified : Boolean (true/false) to indicate the usage of the simplified URDF model.
#   ctl_type: Control type can be 'micro-ros' or 'serial'.
#   motor : Motor rpms to select your motor params (for now, '100' or '1000')
#
ros2 launch orion_description rsp.launch.py
~~~

![rsp_launch](https://github.com/Tesis-ORION/orion_common/blob/main/docs/readmes/rps.gif)

---

## ⚙️ RViz2 Configs

### check_odom.rviz

Visualization oriented to just show the wheels and base links transform, so you can check and validate the odometry, wheel rotation and joint movements when having a real or simulated robot running.

### model_viz.rviz

Full load of TFs and robot model to explore the robot structure, it includes a joint state publisher gui to further explore the joints configuration.

---

## ⚠️ Troubleshooting

### Meshes not loading

Make sure you have sourced the workspace on the terminal you launch the **rsp** or **model_vis**, and also the terminal where you are running the visualization tool.

~~~bash
source ~/ros2_ws/install/setup.bash
~~~

### Parameter not working

This may occur if you send a invalid value in launch param or launch argument which have only certain choices. You can check them by using:

~~~bash
ros2 launch orion_description model_vis.launch.py --show-args
~~~
