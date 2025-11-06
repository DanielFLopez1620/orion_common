# Lasser Assets

This directory contains the drawing files for the parts intended to be 3D printed. They were exported by using **FreeCAD**.

## Preparations

To build your ORION robot, you can print the pieces in materials like PLA, ABS, PETm PETG or similar materials. The versions you can check on the Wiki have mostly used PLA+, but the selection depends on the the endurance you want, the cost and the print time.

The .stl extension files were provided as they are the most common ones that are asked when going to a workshop to print the pieces. If your require other formats, please refer to **[FreeCAdAssets](/orion_assets/FreeCadAssets/README.md)**, search the pieces you need and convert them to the desired format.

### Required pieces to build ORION 1.3.x to 1.6.x

#### Mandatory 3D printed components

The supports and mounts require to be 3D printed because of their form, so you will require the next elements:

|  Name                                                                          | Quantity | Info                                                |
|--------------------------------------------------------------------------------|----------|-----------------------------------------------------|
| [Custom Spacer 10 x 80](/orion_assets/3DPrintAssets/spacer_10x80.stl)         |    16    | Spacer for the levels and joint of walls            |
| [Screen Support Half 1](/orion_assets/3DPrintAssets/screen_support_h1.stl)     |     1    | First half of the screen support                    |
| [Screen Support Half 2](/orion_assets/3DPrintAssets/screen_support_h2.stl)     |     1    | Second half of the screen support                   |
| [Servo Connection](/orion_assets/3DPrintAssets/servo_connection.stl)           |     2    | In case you do not have a metallic one, use this    |
| [Servo Support](/orion_assets/3DPrintAssets/servo_sup.stl)                     |     2    | To mount the servos in charge of the arms           |
| [Simple Arm](/orion_assets/3DPrintAssets/simple_arm.stl)                       |     2    | 1 DoF Arms for non verbal interaction               |
| [Support for A010](/orion_assets/3DPrintAssets/maixsense_sup.stl)              |     1    | Support for Maixsense A010 Depth Camera             |
| [Support for OS30A](/orion_assets/3DPrintAssets/os30a_sup.stl)                 |     1    | Support for YDLIDAR OS30A Detph Camera              |
| [Support for RPi4 Speaker](/orion_assets/3DPrintAssets/speaker_sup.stl)        |     1    | Single speaker support for dual mini speakers       |

The recommended values/properties for 3D printing this parts are:

- **Fill pattern:** Zig zag, triangular or lines.
- **Fill percentage:** 30 % - 50 %.
- **Number of wall layers:** 3 layers.
- **Nozzle width:** 0.4 mm (Except for custom spacer which should be 0.2 mm)
- Accomodate speed and temperature according your material selection.

#### Components based on your selection

You can 3D print the next components too. However, there is an option to cut them as it can be faster and easier due to their planar form. If you want to cut them, check [3DPrintAssets](/orion_assets/LasserAssets/README.md), otherwise, you can print the next elements:

|  Name                                                                      | Quantity | Info                                                |
|----------------------------------------------------------------------------|----------|-----------------------------------------------------|
| [Battery Holder Support](/orion_assets/3DPrintAssets/bathold_support.stl)  |     2    | To mount the battery holders                        |
| [Complete Platform](/orion_assets/3DPrintAssets/com_plate_template.stl)    |     4    | Complete level structure of the robot               |
| [Half Platform](/orion_assets/3DPrintAssets/half_plate_template.stl)       |     2    | Half level structure of the robot                   |
| [Wall Batteries](/orion_assets/3DPrintAssets/wall_batteries.stl)           |     1    | Front wall that allows charger and switch placement |
| [Wall Curve Speaker](/orion_assets/3DPrintAssets/wall_curve_speaker.stl)   |     2    | Curve wall when using RPi4 Speakers                 |
| [Wall Curve](/orion_assets/3DPrintAssets/wall_curve.stl)                   |     2    | Curve wall when using RPi5                          |
| [Wall Front Lidar](/orion_assets/3DPrintAssets/wall_flidar.stl)            |     2    | Front/back wall for LIDAR scanning space            |
| [Wall Lateral Lidar](/orion_assets/3DPrintAssets/wall_llidar.stl)          |     2    | Lateral walls for LIDAR scanning space              |
| [Wall Regulator](/orion_assets/3DPrintAssets/wall_regulator.stl)           |     1    | Back wall that allows check of the regulator        |
| [Wall Raspberry Pi](/orion_assets/3DPrintAssets/wall_rpi.stl)              |     1    | Back wall for RPi HMDIs and aux connections         |
| [Wall Screen](/orion_assets/3DPrintAssets/wall_screen.stl)                 |     2    | Wall that allow the form of the screen              |
| [Wall Servo](/orion_assets/3DPrintAssets/wall_servo.stl)                   |     2    | Lateral wall where the arms are placed              |
| [Wall Stereo](/orion_assets/3DPrintAssets/wall_stereo.stl)                 |     1    | Back wall when using RPi5 speaker                   |

The recommended values/properties for 3D printing this parts are:

- **Fill pattern:** Zig zag, triangular or lines.
- **Fill percentage:** 20 % - 30 %.
- **Number of wall layers:** 3 layers.
- **Nozzle width:** 0.4 mm
- Accomodate speed and temperature according your material selection.

#### Optional 3D prints

You can 3D print the ORION toys, which are cuboids for Computer Vision applications:

|  Name                                                                      | Quantity | Info                                                |
|----------------------------------------------------------------------------|----------|-----------------------------------------------------|
| [ORION Cuboid](/orion_assets/3DPrintAssets/orion_toy_cuboid.stl)           |     1    | Recognition of cubic edges in a cube                |
| [ORION Letters](/orion_assets/3DPrintAssets/orion_toy_letters.stl)         |     1    | Recognition of O, R, I letters in a cube            |
