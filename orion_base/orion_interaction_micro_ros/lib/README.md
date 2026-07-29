# ORION Interaction µ-ROS Libraries

Custom libraries for the ORION interaction firmware running on ESP32 #2.
Handles TFT ILI9225 screen control and emotion rendering via µ-ROS.

## Content

- **[screen](/orion_base/orion_interaction_micro_ros/lib/screen/):** TFT ILI9225
  display driver. Provides bitmap sprite rendering and a geometric fallback for
  the seven ORION emotions (Angry, Disgust, Fear, Happy, Neutral, Sad, Surprise).
  Includes:
  - `screen.hpp` / `screen.cpp` — `Screen` class (`initialize`, `drawEmotion`,
    `drawBitmap`, `displayEmotion`)
  - `emotions.hpp` — bitmap arrays and geometric coordinate constants for each
    emotion face

## Library Structure

```plaintext
lib/
├── screen/
│   ├── emotions.hpp    ← bitmap data + geometric drawing constants (indices 0-6)
│   ├── screen.hpp
│   └── screen.cpp      ← Screen class: TFT init, bitmap and geometric rendering
└── README.md           ← this file
```

## Troubleshooting

### Screen not displaying anything

Verify the HSPI pin assignments in `screen.hpp` match your PCB wiring:

| Signal | Pin                |
|--------|--------------------|
| RST    | 26                 |
| RS     | 25                 |
| CS     | 15                 |
| SDI    | 13                 |
| CLK    | 14                 |
| LED    | 3.3 V (hard-wired) |

### Emotion index out of range

`drawEmotion` accepts indices **0–6** only. Sending a value outside this range
will display no image. The `/emotion/int` topic on the ROS 2 side should enforce
this bound before publishing.

### µ-ROS not compiling

Ensure you have followed the PlatformIO + µ-ROS setup. See
[micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio).
