# ORION Interaction µ-ROS Libraries

Custom libraries for the ORION interaction firmware running on ESP32 #2.
Handles TFT ILI9225 screen control, capacitive touch sensing, and µ-ROS
communication. Follows the same layered pattern as `orion_ctl_micro_ros`
(see [FIRMWARE_REFACTOR_GUIDE.md](/orion_base/FIRMWARE_REFACTOR_GUIDE.md)):

```plaintext
main.cpp (orchestration) → interaction_ctl/ (pure logic) → touch/ + screen/
                          ↘ orion_ros_bridge/ (all ROS, encapsulated)
```

## Content

- **[hardware](/orion_base/orion_interaction_micro_ros/lib/hardware/):**
  GPIO pin definitions for the four capacitive touch sensors
  (`interaction::HARDWARE`).

- **[constants](/orion_base/orion_interaction_micro_ros/lib/constants/):**
  Timing constants — sensor sampling rate and heartbeat interval
  (`interaction::TIMING`).

- **[touch](/orion_base/orion_interaction_micro_ros/lib/touch/):**
  `TouchSensorDriver` — wraps a single capacitive touch pin, with simple
  debounce (`initialize`, `read`, `hasChanged`, `getState`).

- **[screen](/orion_base/orion_interaction_micro_ros/lib/screen/):** TFT ILI9225
  display driver. Provides bitmap sprite rendering and a geometric fallback for
  the eight ORION emotions (Angry, Disgust, Fear, Happy, Neutral, Sad, Surprise,
  Wink).
  Includes:
  - `screen.hpp` / `screen.cpp` — `Screen` class (`initialize`, `drawEmotion`,
    `drawBitmap`, `displayEmotion`)
  - `emotions.hpp` — bitmap arrays and geometric coordinate constants for each
    emotion face

- **[interaction_ctl](/orion_base/orion_interaction_micro_ros/lib/interaction_ctl/):**
  `InteractionController` — pure control logic (no ROS). Composes the four
  `TouchSensorDriver` instances and the `Screen`; samples sensors, tracks the
  current emotion (redraws only on change), and times the heartbeat.

- **[orion_ros_bridge](/orion_base/orion_interaction_micro_ros/lib/orion_ros_bridge/):**
  `interaction_ros.hpp` / `interaction_ros.cpp` — encapsulates ALL micro-ROS
  infrastructure (node, executor, publishers, subscriber, message buffers) in
  an anonymous namespace. Exposes only:
  `interaction_micro_ros_init`, `interaction_micro_ros_publish_touch`,
  `interaction_micro_ros_publish_heartbeat`, `interaction_micro_ros_spin`,
  `interaction_micro_ros_set_emotion_cmd_callback`.

## Library Structure

```plaintext
lib/
├── hardware/
│   └── hardware.hpp              ← touch sensor pin definitions
├── constants/
│   └── constants.hpp             ← sensor rate + heartbeat interval
├── touch/
│   ├── touch_driver.hpp
│   └── touch_driver.cpp          ← TouchSensorDriver: GPIO read + debounce
├── interaction_ctl/
│   ├── interaction_controller.hpp
│   └── interaction_controller.cpp ← InteractionController: pure logic, no ROS
├── orion_ros_bridge/
│   ├── interaction_ros.hpp       ← public micro-ROS interface (5 functions)
│   └── interaction_ros.cpp       ← ROS infrastructure (anonymous namespace)
├── screen/
│   ├── emotions.hpp              ← bitmap data + geometric drawing constants (indices 0-7)
│   ├── screen.hpp
│   └── screen.cpp                ← Screen class: TFT init, bitmap and geometric rendering
└── README.md                     ← this file
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

### Touch sensor reads inverted or on the wrong pad

Verify the GPIO assignments in `hardware.hpp` (`interaction::HARDWARE`) match
your PCB wiring:

| Sensor       | Pin |
|--------------|-----|
| Upper right  | 4   |
| Upper left   | 34  |
| Lower right  | 2   |
| Lower left   | 35  |

### Emotion index out of range

`drawEmotion` accepts indices **0–7** only (Angry, Disgust, Fear, Happy, Neutral,
Sad, Surprise, Wink). `InteractionController::setEmotion()` guards this range and
ignores out-of-range values, so an invalid `/emotion/int` message leaves the
current face on screen instead of reading past the bitmap array.

### µ-ROS not compiling

Ensure you have followed the PlatformIO + µ-ROS setup. See
[micro_ros_platformio](https://github.com/micro-ROS/micro_ros_platformio).

If you see "undefined reference" errors, check for circular includes between
`interaction_ctl/` and `orion_ros_bridge/` — control logic must never include ROS
headers, and the bridge must never include control logic headers. They only
connect through function-pointer callbacks wired up in `main.cpp`.
