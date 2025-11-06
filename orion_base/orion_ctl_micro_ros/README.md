# Actuators code with µ-ROS

## Purpose

Code using two JGA25-371 DC Motors in order to subscribe to a twist topic in order to move. It considers the usage of the L298N driver and the lecture of the two encoder channel of both motors, which aims to connect with a Differentil Controller with ros2_control by using topics. It also connects two servo motors ready for a forward command controller in ros2_control by implementing ROS 2 topics.

This is aimed to mount a code capable for a integration with multiple hardware interfaces of ros2_control (a [DiffDriveController](/orion_control/src/diffdrive_orion.cpp) and two [ForwardCommandControllers](/orion_control/src/forward_orion.cpp)).

NOTE: The motors are connected in a special way in the L298 driver, as OUT1 (+) and OUT2 (-) are for the right motor, and OUT3 (+) and OUT4(-) are for the left motor. Check the **hardware** file on the **lib** directory for more about connections.

A brief note on them is shown below:

- **Motor Left:**
  - **Encoder A:** GPIO32
  - **Encoder B:** GPIO33
  - **Motor Forward (IN3):** GPIO22
  - **Motor Backward (IN4):** GPIO21
  - **Motor Enable (ENB):** GPIO17
- **Motor Right:**
  - **Encoder A:** GPIO34
  - **Encoder B:** GPIO35
  - **Motor Forward (IN3):** GPIO19
  - **Motor Backward (IN4):** GPIO18
  - **Motor Enable (ENB):** GPIO16
- **Servo Right:**
  - **PWM Pin:** GPIO23
- **Servo Right:**
  - **PWM Pin:** GPIO25
- Connect the servos to 5V and GND.
- Ensure that the L298N doesn't use the 12V to power on, as you will use the 5V from the ESP32 to power it on.
- The battery stack should provide between 9.6 V to 12.6 V, although this, make sure to connect this output to the DC motors supply at the L298N.

For more information about the connections, check the [ORION Wiki](https://github.com/Tesis-ORION/orion_common/wiki/Building-your-own-ORION-robot#electronics-and-schematics)

## Uploading and running application

1. Prepare thw **Platformio** workspace: Install dependencies, build and upload

    ~~~bash
    cd /path/to/orion_ctl_micro_ros
    pio lib install
    pio run
    pio run --target upload
    ~~~

    Do not forget to check the **udev** rules at [orion_base](/orion_base/README.md)

2. Go to your workspace

    ~~~bash
    cd ~/ros2_ws

    # If you do not have one, create it
    mkdir -p ~/ros2_ws/src
    cd ~/ros2_ws
    ~~~

3. Clone the µ-ROS repository, update and install dependencies, if you haven't done it before.

    ~~~bash
    git clone -b $ROS_DISTRO https://github.com/micro-ROS/micro_ros_setup.git src/micro_ros_setup
    sudo apt update && rosdep update
    rosdep install --from-paths src --ignore-src -y
    sudo apt-get install python3-pip
    ~~~

4. Build your workspace.

    ~~~bash
    colcon build
    source ~/ros2_ws/install/local_setup.bash
    ~~~

5. Create a firmware workspace, if you haven't done it before.

    ~~~bash
    ros2 run micro_ros_setup create_firmware_ws.sh freertos esp32
    ~~~

6. Create the agent, if you haven't.

    ~~~bash
    ros2 run micro_ros_setup create_agent_ws.sh
    ~~~

7. Build the agent if you haven't.

    ~~~bash
    ros2 run micro_ros_setup build_agent.sh
    source install/local_setup.bash
    ~~~

8. Give the proper permissions to the device.

    ~~~bash
    sudo chmod 777 /dev/ttyESP32_1
    sudo chmod 666 /dev/ttyUSB0 # Or the proper device
    ~~~

9. You are ready to experiment with the application.

    ~~~bash
    ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyESP32_1
    ~~~

## Topics

Once the ESP32 is connected and the agent is communicating with it, you should watch the topics that are publishing/subscribing info of the motors (DC motors and servo motors). Let's explore them:

~~~bash
ros2 topic list -t
~~~

So you should watch the next topics:

~~~bash
/diff_ctl_left_enc (std_msgs/msg/Int64)
diff_ctl_motor_cmd (std_msgs/msg/Int64MultiArray)
/diff_ctl_right_enc (std_msgs/msg/Int64)
/fwd_servo_left_cmd (std_msgs/msg/Float32)
/fwd_servo_left_feedback (std_msgs/msg/Float32)
/fwd_servo_right_cmd (std_msgs/msg/Float32)
/fwd_servo_right_feedback (std_msgs/msg/Float32)
~~~

With this, you can do the next

- You can check the encoder count changes per wheel, For this, on separated terminals, subscribe with **ros2 topic echo**:

    ~~~bash
    # Terminal 1
    ros2 topic echo /diff_ctl_left_enc

    # Terminal 2
    ros2 topic echo /diff_ctl_right_enc
    ~~~

    Start moving the motors manually and check the increment/decrement of both, based on the front (positive) and back (negative) directions. If they are inverted check connections or review ports on code ([harwdare.hpp](/orion_base/orion_ctl_micro_ros/lib/hardware/hardware.hpp))

- You can command the motors by using a muti array topic as shown below:

    ~~~bash
    ros2 topic pub /diff_ctl_motor_cmd std_msgs/msg/Int64MultiArray "layout:
    dim: []
    data_offset: 0
    data: [0,0]" --once
    ~~~

    The data in the message should refer to the encoder count change in position you require to do in a time lapse, this may vary depending of your motor as a 1000 rpm motor may have less encoder counts than a 100 rpm motor due to the reductor configuration.

- You can write a servo position by using:

    ~~~bash
    # Servo angles must be passed as radians
    ros2 topic pub /fwd_servo_left_cmd std_msgs/msg/Float32 "data: 1.0"
    ros2 topic pub /fwd_servo_right_cmd std_msgs/msg/Float32 "data: 1.0"
    ~~~

    And you can read the servo position by using:

    ~~~bash
    # Terminal 1
    ros2 topic echo /fwd_servo_left_feedback
    # Terminal 2
    ros2 topic echo /fwd_servo_right_feedback
    ~~~

## Additional notes

- Do not run the agent while the bringup is active, as you may cause to interrupt it.

- Avoid to publish to the DC motors and servo motors topics when using the proper ros2_controllers as you may affect other nodes processes.

## Additional resources

- [FreeRTOS | µ-ROS](https://micro.ros.org/docs/tutorials/core/first_application_rtos/freertos/)

- [ESP32 with DC Motor and L298N Motor Driver – Control Speed and Direction | Random Nerd Tutorials](https://randomnerdtutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/)

- [talker_c | riot-ros2 @ Github](https://github.com/astralien3000/riot-ros2/blob/3d0779b920996f4e701830b8248573cd0e23204d/examples/talker_c/main.c#L32)

- [micro_ros_platfomio | micro-ROS @ Github](https://github.com/micro-ROS/micro_ros_platformio)
