# Interaction base code with µ-ROS

## Purpose

Code oriented to implement the TFT ILI9225 Screen and four capacitive touch sensors with a µ-ROS application, in order to use these elements for HRI behaviors with the ORION robot.

Let's get an overview on the connections:

- **Screen:** TFT ILI9225
  - **VCC:** 5V
  - **GND:** GND
  - **GND:** GND
  - **NC:** Not connected
  - **NC:** Not connected
  - **LED:** 3.3V
  - **CLK:** GPIO14 (HSPI-SCK)
  - **SDI:** GPIO13 (HSPI-MOSI)
  - **RS:** GPIO25
  - **RST:** GPIO26
  - **CS:** GPIO15 (HSPI-SSO)
- **Touch sensors:**
  - **Upper Right:** GPIO4
  - **Upper Left:** GPIO34
  - **Lower Right:** GPIO2
  - **Lower Left:** GPIO35
  - Connect all of the touch sensors to 5V and GND.

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

Once the ESP32 is connected and the agent is communicating with it, you should watch the topics that are publishing/subscribing info of the touch sensors and the screen. Let's explore them:

~~~bash
ros2 topic list -t
~~~

So you should watch the next topics:

~~~bash
/interaction/touch_ur (std_msgs/msg/Bool)
/interaction/touch_ul (std_msgs/msg/Bool)
/interaction/touch_lr (std_msgs/msg/Bool)
/interaction/touch_ll (std_msgs/msg/Bool)
/emotion/int (std_msgs/msg/Bool)
~~~

With this, you can do the next:

- You can check the touch sensor states, For this, on separated terminals, subscribe with **ros2 topic echo**:

    ~~~bash
    # Terminal 1
    ros2 topic echo /interaction/touch_ur

    # Terminal 2
    ros2 topic echo /interaction/touch_ul

    # Terminal 3
    ros2 topic echo /interaction/touch_lr

    # Terminal 4
    ros2 topic echo /interaction/touch_ll
    ~~~

    Start the usage of the touch sensors, they should send true (1) when being in contact with your hand. If they are inverted or disorganized, check connections or review ports on the code definitions on ([main.cpp](/orion_base/orion_interaction_micro_ros/src/main.cpp))

- You can command the screen by publishing to the emotion topic, as this will display a face according to this:

    ~~~bash
    ros2 topic pub /emotion/int std_msgs/msg/Int64 "data: 2"
    ~~~

    For now there are seven emotions which are: Angry (0), Disgust (1), Fear(2), Happy (3), Neutral (4), Sad (5), Surprise (6).

## Additional notes

- The emotion topic is related with the [emotion_detector](https://github.com/Tesis-ORION/emotion_detector) package, check it for more information.

## Additional resources

- [FreeRTOS | µ-ROS](https://micro.ros.org/docs/tutorials/core/first_application_rtos/freertos/)

- [TFT_22_ILI9225 Screen Wiki](https://github.com/Nkawu/TFT_22_ILI9225/wiki)

- [talker_c | riot-ros2 @ Github](https://github.com/astralien3000/riot-ros2/blob/3d0779b920996f4e701830b8248573cd0e23204d/examples/talker_c/main.c#L32)

- [micro_ros_platfomio | micro-ROS @ Github](https://github.com/micro-ROS/micro_ros_platformio)
