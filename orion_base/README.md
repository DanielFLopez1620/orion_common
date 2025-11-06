# 🤖 ORION Base

## 🌟 Overview

This directory contians the embedded codes for the **ESP32**s of the **ORION** robot. They were build by using [PlatformIO](https://platformio.org/) and C/C++.

To check the robot electronic connections, review the [Electronics and Schmatics](https://github.com/Tesis-ORION/orion_common/wiki/Building-your-own-ORION-robot#electronics-and-schematics) on the Wiki.

---

## 📝 License

The source code is released under a [BSD 3-Clause license](/LICENSE).

**Maintainer**: [Daniel Felipe López Escobar](https://github.com/DanielFLopez1620).

The ORION Commons packages have been tested under [ROS](https://www.ros.org/) **Jazzy** distribution.

---

## 📚 Table of Contents

- [⚡ µ-ROS codes](#-µ-ros-codes)
- [⚡ Serial codes](#-serial-codes)
- [⚠️ Trouble shooting](#️-trouble-shooting)

## ⚡ µ-ROS codes

The usage of [µ-ROS](https://micro.ros.org/) allows a microcontroller, like a ESP32, to be able to work on the ROS 2 ecosystem by using a agent for the communciation, which give the opportunity to work with topics, publishers, subscriber, services, nodes, among others on embedded devices.

For ORION, it has a two ESP32 application, shown as:

- **Actuators:** The ESP32 located on the first floor of the robot and that is in charge of the control of the DC motors and servomotors, which is intended to be a base for communicating with a hardware interface of ros2_control that are defined on [orion_control](/orion_control/README.md). For implementing this, check out the [orion_ctl_micro_ros](/orion_base/orion_ctl_micro_ros/README.md) directory.

- **Interaction:** The ESP32 on the top floor of the robot manage the screen and touch sensors which are mainly focused on interaction tasks with users. For the implementation, check out the [orion_interaction_micro_ros](/orion_base/orion_interaction_micro_ros/README.md) directory.

### Uploading the codes

For the embedded codes, it is required to use **PlatformIO** in your development machine (it can be your Raspberry Pi or you PC), consider the next:

- **[VSCode and Platformio Extension](https://docs.platformio.org/en/stable/integration/ide/vscode.html):** You can use it as an extension in VS Code, it will also install the dependencies and CLI tools, do not forget to check that you have **venv** installed for using the Python Virtual Environments.

- **CLI Tools only**: If you prefer a terminal or you are using a RPi with a server installation for Ubuntu, you can proceed with the next steps:

  1. Make sure **pip** and **venv** are installed:

        ~~~bash
        sudo apt install python3-pip python3-venv
        ~~~

  2. Obtain the installation script:

        ~~~bash
        wget -O get-platformio.py https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py
        ~~~

  3. Execute the installation script:

        ~~~bash
        python3 get-platformio.py
        ~~~

With this ready, let's proceed to specify the target of the installation:

1. Connect both ESP32 to the RPi, determinate which device at **/dev** is the ESP32 for actuators and the ESP32 for interaction, for example, */dev/ttyUSB0* and */dev/ttyUSB1* respectively.

2. Identify the attributes of the connection with:

    ~~~bash
    # Terminal 1
    udevadm info --name=/dev/ttyUSB0 --attribute-walk
    # Terminal 2
    udevadm info --name=/dev/ttyUSB1 --attribute-walk
    ~~~

    And search for elements like:

    ~~~bash
    ATTRS{idVendor}=="10c4"
    ATTRS{idProduct}=="ea60"
    ATTRS{serial}=="0001"
    ~~~

    If you do not find them, as the output can be very long, you can use grep:

    ~~~bash
    udevadm info --name=/dev/ttyUSB0 --attribute-walk | grep idVendor
    ~~~

3. Create a **udev** rule file and proceed to add the elements:

    ~~~bash
    sudo nano /etc/udev/rules.d/99-usb-serial.rules
    ~~~

    Inside of the file, add the next (based on your outputs):

    ~~~rules
    SUBSYSTEM=="tty", ATTRS{idVendor}=="10c4", ATTRS{idProduct}=="ea60", ATTRS{serial}=="0000", SYMLINK+="ttyESP32_1"
    SUBSYSTEM=="tty", ATTRS{idVendor}=="10c5", ATTRS{idProduct}=="ea61", ATTRS{serial}=="0001", SYMLINK+="ttyESP32_2"
    ~~~

4. (Optional) If your attributes are the same for both ESP32 (probably they are from the same production slot), do not worry, you can use environmental attributes, this isn't recommended as they may change if you change the ESP32 of position or connect all the devices different, so keep this in mind. For this you can use:

    ~~~bash
    udevadm info --name=/dev/ttyUSB0 | grep ID_PATH
    udevadm info --name=/dev/ttyUSB1 | grep ID_PATH
    ~~~

    Then create custom rules for this purpose, like:

    ~~~rules
    SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-fd500000.pcie-pci-0000:01:00.0-usb-0:1.3.1:1.0", SYMLINK+="ttyESP32_1"
    SUBSYSTEM=="tty", ENV{ID_PATH}=="platform-fd500000.pcie-pci-0000:01:00.0-usb-0:1.3.3:1.0", SYMLINK+="ttyESP32_2"
    ~~~

    **NOTE:** Avoid to generate rulse that may end adding the same name / symlink to multiple devices as this can mess up the robot software and the system of the RPi itself.

5. Update the udev rules.

    ~~~bash
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ~~~

6. Check that the rules have been updated.

    ~~~bash
    ls -al /dev | grep USB
    # Should output the /dev/ttyUSB devices and the symbolic links for the ESP32_1 and ESP32_2
    ~~~

Once the ESP32 naming has been done, we can proceed with the upload:

1. Give the proper permissions to the devices.

    ~~~bash
    sudo chmod 777 /dev/ttyESP32_*
    sudo chmod 666 /dev/ttyUSB0 # Or your first ESP32 port
    sudo chmod 666 /dev/ttyUSB1 # Or your second ESP32 port
    ~~~

2. Go to the **PlatformIO** workspace of you application.

    ~~~bash
    # In case you are using VS Code
    code /path/to/orion_ctl_micro_ros
    # or
    code /path/to/orion_interaction_micro_ros
    ~~~

    ~~~bash
    # In case you just want the terminal
    cd /path/to/orion_ctl_micro_ros
    # or
    cd /path/to/orion_itneraction_micro_ros
    ~~~

3. Install the depdencies.

    In the case of VS Code, go to the **platformio.ini** file and save (Ctrl+S), it should start the configuration of the *PlatformIO* extension.

    ~~~bash
    # In case of terminal, use:
    pio lib install
    # or
    platformio install
    ~~~

4. Build the content.

    In the case of VS Code, press the **Build** button.

    ~~~bash
    # In case of terminal, use:
    pio run
    # or
    platformio run
    ~~~

5. If the building is successfully completed, upload (it will use the targets on the **platformio.ini). Otherwise, review the error before uploading the code.

    In the case of VS Code, press the **Upload** button.

    ~~~bash
    # In case of terminal, use
    pio run --target upload
    # or
    platformio run --target upload
    ~~~

    **NOTE:** You may require to pulse the **BOOT** button if the upload seems to get stuck when recognizing the device.

    ![esp32_buttons](https://private-user-images.githubusercontent.com/49737722/290580785-1cb9e118-9004-4f53-9843-e682e02711fb.png?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDg4MTUwNDksIm5iZiI6MTc0ODgxNDc0OSwicGF0aCI6Ii80OTczNzcyMi8yOTA1ODA3ODUtMWNiOWUxMTgtOTAwNC00ZjUzLTk4NDMtZTY4MmUwMjcxMWZiLnBuZz9YLUFtei1BbGdvcml0aG09QVdTNC1ITUFDLVNIQTI1NiZYLUFtei1DcmVkZW50aWFsPUFLSUFWQ09EWUxTQTUzUFFLNFpBJTJGMjAyNTA2MDElMkZ1cy1lYXN0LTElMkZzMyUyRmF3czRfcmVxdWVzdCZYLUFtei1EYXRlPTIwMjUwNjAxVDIxNTIyOVomWC1BbXotRXhwaXJlcz0zMDAmWC1BbXotU2lnbmF0dXJlPTU5NjE3NzE1MzM2ZDhmMWFmNTVhNDQ3YmRlZDEzYTkwNGMwYTY5NmEzYmU3ZTc0OWNlNjgwZTBmOWI0ZDVhN2UmWC1BbXotU2lnbmVkSGVhZGVycz1ob3N0In0.ixwxohKsnIo93JzucE2aICNdrre7JA__tYV4RiGf-qQ)

### Known issues

- **µ-ROS agent not linking with the program:** With the implementation provided, the ESP32 waits up to 2 minutes to connect with the µ-ROS agent. Otherwise, it will continue and will require a manual restart by using the **EN** button on the ESP32 which may cause problems as you may need to dismoun some of the robot walls to access to it. You can use the following alternatives:

  - Disconnect the ESP32 while the agent is active, when it restart it may be able to connect again.
  - Close the agent, reupload the code to the ESP32 and open the agent again.

- **ROS_DOMAIN_ID != 0:** You communication with µ-ROS may not be possible if you are using a different **ROS_DOMAIN_ID** as it uses the default value (0) for it.

## ⚡ Serial codes

This implementation aims for the usage of serial communication with ESP32 as an alternative to the usage of **µ-ROS**.

This will be updated in the future.

## ⚠️ Trouble shooting

### Screen flickering

This may be caused by two reasons:

- **NC pin that requires connection:** In some models of the TFT ILI9225 Screen there is a **NC** (Not connected) pin that is really a **LED** pin which is related to the brightness of the screen, and if it is not connected it will produce the flickering.

- **Bad connection of the screen:** Another reason can be a bad connection on the SPI ports when loading the code, as it may lead to send the information in the wrong pins, so double check the connection. Also, if you connected the screen in a wrong way, disconnect it and try it alone to check it still works as if you messed with the power supply ports it may broke.

You can check more info of the screen on [TFT 22 ILI9225 Wiki](https://github.com/Nkawu/TFT_22_ILI9225/wiki).

### Robot moving the in the wrong direction

This may be caused by the wiring of the motor according to its manufacturer or a mismatch in the cable connection. However, you do not require to dissambly the robot, you can go to the [hardware constants](/orion_base/orion_ctl_micro_ros/lib/hardware/hardware.hpp) and exchange the values.

For thsi purpose try to exchange the values of the **Forward** and **Backward** pins, do the test for each wheel individually to ensure the movement, you can even run the µ-ROS code and use the instructions in [orion_ctl_micro_ros](/orion_base/orion_ctl_micro_ros/README.md) to move the motors manually.

### Feedback of wheel rotation is inverse

Teh encoder lecture should increase when the wheels move forward, and dicrease otherwise. If the direction is wrong when checking RViz2 while the **ros2_controllers** are active or you read the output of the encoder of each motor and it is not the expected increment/decrement, go to [hardware constants](/orion_base/orion_ctl_micro_ros/lib/hardware/hardware.hpp) and exchange the encoder channels (A and B) pins and do a test.

### Robot is going too fast

If the robot goes really fast it can mess the SLAM applications and Navigation, in case the default velocity PWM limit isn't enough, you can manually change it in the file [motor.hpp](/orion_base/orion_ctl_micro_ros/lib/motor/motor.hpp) in the constant attribute **MAX_SPEED**.

### /dev/ttyESP32_n not found

This is caused as the ESP32 connection wasn't updated with the proper udev rules, check this above as the names managed are used in the bringup, the robot startup service and PlatformIO set ups.

The reasons you can consider for this are:

- You changed the eSP32 and the attributes are therefore differnt.
- You are using environmental attributes, like ID_PATH, and changed the order connecttions.
- The device isn't connected, so the udev rules isn't applied. Check the cable and the element in case they are damaged.

### /dev/ttyESP32_2 not uploading program

One possible reasons is the connections on the ESP32_2 (for interaction purposes), as the *lower right* touch sensor is mounted on the GPIO2. If the sensor is pressed while uploading the program, it may fail as in the documentation of the ESP32 Doit Dev Kit you have to avoid HIGH values on that pin in startup / uploading.

More info on [ESP32 Pinout | Last Minute Engineering](https://lastminuteengineers.com/esp32-pinout-reference/)
