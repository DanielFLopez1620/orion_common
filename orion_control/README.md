# 🤖 ORION Control

## 🌟 Overview

Package oriented to the control configuration and parameters for the ORION robot, it also includes the sources and headers for the hardware interfaces for the ORION mobile base controller and the ORION simple arms forward command controllers.

---

## 📝 License

The source code is released under a [BSD 3-Clause license](/orion_control/LICENSE.md).

**Author**: Daniel Felipe López Escobar.

The *orion_control* package has been tested under [ROS](https://www.ros.org/) Jazzy.

---

## 📚 Table of Contents

- [⌨️ Source Codes](#️-source-codes)
- [⌨️ Plugins](#️-plugins)
- [⚙️ Params and Configs](#️-params-and-configs)
- [⚠️ Troubleshooting](#️-troubleshooting)

---

## ⌨️ Source Codes

You can find the next codes:

- [diffdrive_orion](/orion_control/src/diffdrive_orion.cpp): Implementation of the hardware interface for a **ros2_control** *Diff Drive Controller*. It is focused on the implementation for the read/write methods based on the topics of [orion_ctl_µ_ros](/orion_base/orion_ctl_micro_ros/README.md) for the DC motors, which are based on a PID controller (for each motor) by using the double encoder count to determinate speed and direction, while also doing the control loop.

- [forward_orion](/orion_control/src/forward_orion.cpp): Implementation of the hardware interface for a **ros2_control** *Forward Command Controller*. It is focused on the read/write methods on the topics from [orion_ctl_µ_ros](/orion_base/orion_ctl_micro_ros/README.md) for the servo motors, where it has write/read position (default) and an option to use incremental moves (can be changed in the commented notes of the servo callback).
- [wheel](/orion_control/src/wheel.cpp): Wheel object used during the DiffDrive Controller, mainly focused on wheel params like encoder counts per revolution and methods for this conversion.

---

## ⌨️ Plugins

With [ctl_orion.xml](/orion_control/ctl_orion.xml), there are two plugins that are loaded to the ROS 2 Workflow:

- **orion_control/DiffDriveOrion**: Oriented to the mobile base of ORION.

- **orion_control/ForwardOrion**: Oriented to the simple arms of ORION.

Both are called on the description of the robot when using ROS 2. If you intend to change the mobile base (for example, design a holonomic one or a sterring wheel) and the arms (for example, include more articulations), keep in mind that you may require to add an additional plugin at this package.

---

## ⚙️ Params and Configs

In this packages you can find the configuration for the controllers of the robot, they work for simulation and physical robot:

- **[control_manager.yaml](/orion_control/config/control_manager.yaml):**  Define the base params for the real robot controller manager: **update_rate** and **sim_time**.
- **[g_mov_servo_controller.yaml](/orion_control/config/g_mov_servo_controller.yaml):** Adds a Forward Controller for the G-Mov servo to control position.
- **[gz_ros_control.yaml](/orion_control/config/gz_ros_control.yaml):** Focused on the GZ Sim configuration for the controller manager.
- **[joint_state_broadcaster.yaml](/orion_control/config/joint_state_broadcaster.yaml):** Aims to configure the Joint Broadcaster that considers the changes of all the joints defined with **ros2_control.**
- **[mobile_base_controller.yaml](/orion_control/config/mobile_base_controller.yaml):** Add a Diff Drive Controller with its parameters for the ORION mobile base.
- **[simple_left_arm_controller.yaml](/orion_control/config/simple_left_arm_controller.yaml):** Set up the parameters for the Forward Command Controller of the left arm.
- **[simple_right_arm_controller.yaml](/orion_control/config/simple_right_arm_controller.yaml):** Set up the parameters for the Forward Command Controller of the right arm.

If you add more controllers to the robot, do not forget to add them to the **config** dir of the package.

---

## ⚠️ Troubleshooting

### Small deviations over time with odometry

You can also confirm this by moving the wheel of your robot manually and you note a little deviation over time, for example, 20° over 20 laps.

This may be caused by a small difference in the wheel size and wheel separation. To fix this, you can measure your robot and change the params of [mobile_base_controller.yaml](/orion_control/config/mobile_base_controller.yaml).

Additionally you can check the encoder count of your robot, for more information check on [orion_base](/orion_base/README.md) and the next point on this section.

### Big deviation over small periods of time with odometry

You can also confirm by spinning the wheel of your robot and if the wheel spin too much or too little based on the movement, it may be a problem of the encoder count and wheel definition in your URDF.

To solve this, go to [orion.urdf.xacro](/orion_description/urdf/orion.urdf.xacro) and change the **motor** params. If you are using a JGA with a reductor for an output of 1000 rpm approx at 12V, use **1000**; if you are using a JGP with a output of 100 rpm approx at 12V use **100**; and if you aren't using either of these, you will need to create a custom one.

To create a custom encoder count and motor params do the next:

1. Go to [orion.urdf.xacro](/orion_description/urdf/orion.urdf.xacro) and search for the motor launch param / config, edit the choices to allow your motor nominal speed at 12V.

    ~~~Python
    DeclareLaunchArgument('motor', default_value='100',
        description="Select your  motor nominal speed (rpm) at 12V",
        choices=['1000', '100', '<add_yours>']),
    ~~~

2. Go to [orion.urdf.xacro](/orion_description/urdf/orion.urdf.xacro) and validate that your **motor** accepts args as a param.

    ~~~XML
    <xacro:property name="motor" value="$(arg motor)"/>
    ~~~

3. Go to [orion_macros.urdf.xacro](/orion_description/urdf/include/orion_macros.urdf.xacro), search for the continuous joint definition and add a conditional case for your motor.

    ~~~XML
    <joint name="coupling_${front}${side}_joint" type="continuous">
        <axis xyz="0.0 0.0 1.0"/>
        <parent link="motor_support_${front}${side}"/>
        <child link="coupling_${front}${side}"/>
        <xacro:if value="${motor == 1000}">
            <limit effort="0.01079" velocity="104.97"/>
        </xacro:if>
        <xacro:if value="${motor == 100}">
            <limit effort="0.08336" velocity="10.47"/>
        </xacro:if>
        <xacro:if value="${motor == <add_yours>}">
            <limit effort="<add_yours>" velocity="<add_yours>"/>
        </xacro:if>
        <origin xyz="0 ${(0.01120 + 0.000325)*muls} 0" rpy="${-pi_2*muls+orin} 0 0"/>
    </joint>
    ~~~

4. Go to [orion_ros2_control.urdf.xacro](/orion_description/urdf/include/orion_ros2_control.urdf.xacro), search for Diff Drive controller and update the encoder counts with an additional case for you motor.

    ~~~XML
    <hardware>
        <plugin>orion_control/DiffDriveOrion</plugin>
        <param name="left_wheel_name">coupling_11_joint</param>
        <param name="right_wheel_name">coupling_12_joint</param>
        <xacro:if value="${motor == 1000}">
            <param name="enc_ticks_per_rev">204</param>
        </xacro:if>
        <xacro:if value="${motor == 100}">
            <param name="enc_ticks_per_rev">1250</param>
        </xacro:if>
        <xacro:if value="${motor == <add_your>}">
            <param name="enc_ticks_per_rev">'add_yours'</param>
        </xacro:if>
    </hardware>
    ~~~

5. You may additioally change your max and min PWM for the real robot, for more information, check on [orion_base](/orion_base/README.md)
