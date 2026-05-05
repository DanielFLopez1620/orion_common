# RPi Camera v1.3 (OV5647) on Ubuntu Server 24.04 — Raspberry Pi 4
### For use with `camera_ros` in ROS 2 Jazzy

---

## Background

The **RPi Camera Module v1.3** uses the **OV5647** sensor. Getting it to work on Ubuntu Server 24.04 and connecting it to ROS 2 Jazzy requires several non-obvious steps, mainly because:

- Ubuntu 24.04 ships an *upstream* version of `libcamera` that **has no Raspberry Pi hardware support**
- The `rpicam-*` binaries do not come from `libcamera` — they live in a separate repo (`rpicam-apps`)
- The `libav` encoder bundled in `rpicam-apps` is incompatible with the FFmpeg version in Ubuntu 24.04
- `/dev/dma_heap` permissions block access for non-root users without extra configuration
- `ros-jazzy-camera-ros` installed via apt ships its own `libcamera` that **conflicts** with the RPi fork built from source, causing IPA proxy crashes
- The OV5647 native encoding (`NV21`) is not compatible with RViz or rqt_image_view without explicit parameters

---

## Prerequisites

- Raspberry Pi 4
- Ubuntu Server 24.04 (64-bit, aarch64)
- ROS 2 Jazzy installed at `/opt/ros/jazzy`
- RPi Camera Module v1.3 connected to the CSI port (ribbon cable with metal contacts facing the HDMI ports)
- Internet access from the RPi

---

## Step 1 — Physical connection

Connect the camera to the **CSI port** on the RPi 4. The ribbon cable must be inserted with the metal contacts facing the HDMI ports. Gently pull the black clip, insert the cable straight, and press the clip back until it clicks.

> ⚠️ **Maximum recommended ribbon length:** up to 50 cm for reliable operation. Longer cables may work but are prone to CSI signal failures (sensor responds over I2C but frames time out). For mobile robots, consider Arducam CSI-to-HDMI extenders that support up to 10 m.

---

## Step 2 — Configure `/boot/firmware/config.txt`

```bash
sudo nano /boot/firmware/config.txt
```

Add or modify:

```ini
# Disable auto-detection (causes conflicts with OV5647 on Ubuntu)
camera_auto_detect=0

# Load the OV5647 sensor overlay
dtoverlay=ov5647
```

> ⚠️ The correct file on Ubuntu 24.04 is `/boot/firmware/config.txt`, **not** `/boot/config.txt`.
> With `camera_auto_detect=1` and no explicit dtoverlay, `/dev/video0` is not created correctly.

Reboot:

```bash
sudo reboot
```

---

## Step 3 — Verify kernel detection

```bash
ls -l /dev/video*
v4l2-ctl --list-devices
sudo dmesg | grep -i -E "camera|ov5647|unicam|csi"
```

Expected dmesg output:

```
# Sensor detected correctly — "i2c read error" must NOT appear
/soc/csi@7e801000: Fixed dependency cycle(s) with /soc/i2c0mux/i2c@1/ov5647@36
```

> ⚠️ If you see `ov5647_read: i2c read error, reg: 300a = -5` → the sensor is not responding over I2C. Check the physical cable connection before continuing.

---

## Step 4 — Remove system libcamera (Ubuntu version, no RPi support)

Ubuntu 24.04 installs `libcamera 0.2.x` by default — the upstream version **without Raspberry Pi support**. It must be removed before building the RPi fork to prevent linking conflicts.

```bash
sudo apt remove --purge libcamera-dev libcamera0.2 -y
sudo ldconfig
```

Verify no leftovers remain:

```bash
ldconfig -p | grep libcamera
# Should show nothing, or only paths under /usr/local/
```

---

## Step 5 — Install build dependencies

```bash
sudo apt update && sudo apt full-upgrade -y
sudo apt install -y \
  git cmake meson ninja-build build-essential \
  python3-pip python3-yaml python3-ply \
  libboost-dev libboost-program-options-dev \
  libgnutls28-dev openssl \
  libjpeg-dev libtiff5-dev libpng-dev \
  libdrm-dev libexpat1-dev \
  libepoxy-dev libexif-dev \
  libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libswresample-dev \
  libgles2-mesa-dev \
  v4l-utils
```

> ⚠️ `libgles2-mesa-dev` provides the `GLES2/gl2.h` headers required by libcamera's EGL module. Without it, the build fails with `fatal error: GLES2/gl2.h: No such file or directory`.

---

## Step 6 — Build libcamera (Raspberry Pi fork)

> ⚠️ **RAM warning:** compilation requires ~1.5 GB of free memory. Use `-j2` to limit parallel jobs and prevent the OOM killer from terminating `cc1plus`. If free RAM is below 1 GB, create a temporary swap file first:
> ```bash
> sudo fallocate -l 2G /swapfile && sudo chmod 600 /swapfile
> sudo mkswap /swapfile && sudo swapon /swapfile
> ```

```bash
cd ~
git clone https://github.com/raspberrypi/libcamera.git
cd libcamera
meson setup build
ninja -C build -j2
sudo ninja -C build install
```

Register the installed libraries:

```bash
echo "/usr/local/lib/aarch64-linux-gnu" | sudo tee /etc/ld.so.conf.d/rpicam.conf
sudo ldconfig
```

Verify:

```bash
ldconfig -p | grep libcamera
# Must show entries pointing ONLY to /usr/local/lib/aarch64-linux-gnu
```

---

## Step 7 — Build rpicam-apps

`rpicam-hello`, `rpicam-still`, `rpicam-vid` etc. live in a **separate repo** from `libcamera`.

> ⚠️ **Required omission:** The `-Denable_libav=disabled` flag is necessary because `libavcodec` on Ubuntu 24.04 (v60.x) is incompatible with the `rpicam-apps` encoder. This has no impact on `camera_ros` usage in ROS 2.

```bash
cd ~
git clone https://github.com/raspberrypi/rpicam-apps.git
cd rpicam-apps
meson setup build --buildtype=release -Denable_libav=disabled
ninja -C build -j2
sudo ninja -C build install
sudo ldconfig
```

Verify binaries:

```bash
which rpicam-hello
rpicam-hello --version
```

---

## Step 8 — Configure dma_heap permissions

Without this step, `rpicam-hello` and `camera_ros` fail with `Could not open any dmaHeap device` for non-root users.

```bash
# Add user to video group
sudo usermod -aG video $USER

# Create udev rule
echo 'SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"' | \
  sudo tee /etc/udev/rules.d/99-dma-heap.rules

# Apply rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

**Log out and reconnect via SSH** for the group change to take effect:

```bash
exit
# reconnect...
groups  # must include "video"
```

---

## Step 9 — Verify libcamera

```bash
rpicam-hello --list-cameras
```

Expected output:

```
Available cameras
-----------------
0 : ov5647 [2592x1944] (/base/soc/i2c0mux/i2c@1/ov5647@36)
    Modes: 'SGBRG10_CSI2P' : 640x480 [58.92 fps] ...
```

Capture test:

```bash
rpicam-still -o /tmp/test.jpg

# Transfer to PC to verify visually:
# (from PC) scp ubuntu@<RPI_IP>:/tmp/test.jpg ~/Desktop/
```

---

## Step 10 — Set up camera_ros for ROS 2 Jazzy

### 10.1 — Remove ros-jazzy-libcamera (critical conflict)

`ros-jazzy-camera-ros` installed via apt pulls its own `libcamera` into `/opt/ros/jazzy/lib/`. This version is **ABI-incompatible** with the RPi fork built from source, causing IPA proxy crashes:

```
FATAL Serializer control_serializer.cpp:626 A list of V4L2 controls requires a ControlInfoMap
ERROR IPAProxy raspberrypi_ipa_proxy.cpp:316 Failed to call start: -110
```

The root cause: the `raspberrypi_ipa_proxy` built from source and the apt `libcamera.so` use different serialization protocol layouts — even if both report version "0.7.0", they are different builds with incompatible ABI.

Remove the ROS libcamera:

```bash
sudo apt remove --purge ros-jazzy-libcamera ros-jazzy-camera-ros -y
sudo ldconfig

# Verify it no longer exists in the ROS path
ls /opt/ros/jazzy/lib/libcamera.so* 2>/dev/null
ls /opt/ros/jazzy/lib/libcamera-base.so* 2>/dev/null
# Both must return empty

# Confirm ldconfig points only to /usr/local/
ldconfig -p | grep "libcamera.so"
# Must show only /usr/local/lib/aarch64-linux-gnu/ entries
```

> ⚠️ The files `libcamera_calibration_parsers.so` and `libcamera_info_manager.so` that remain in `/opt/ros/jazzy/lib/` are ROS packages with similar names but **completely unrelated** to libcamera — do not remove them.

### 10.2 — Install camera-info and ROS dependencies

```bash
sudo apt install -y \
  ros-jazzy-camera-info-manager \
  ros-jazzy-image-transport \
  ros-jazzy-image-transport-plugins
```

### 10.3 — Set environment variables

```bash
cat >> ~/.bashrc << 'EOF'
# libcamera RPi fork — prevent conflicts with apt/ROS version
export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera
EOF
source ~/.bashrc
```

### 10.4 — Build camera_ros from source

The apt package links against the ROS libcamera (incompatible). It must be built from source to use the RPi fork:

```bash
# Create workspace
mkdir -p ~/ros2_ws/src && cd ~/ros2_ws

# Clone camera_ros
git clone https://github.com/christianrauch/camera_ros.git src/camera_ros

# Build pointing to /usr/local where the RPi fork lives
source /opt/ros/jazzy/setup.bash
colcon build --packages-select camera_ros \
  --cmake-args -DCMAKE_PREFIX_PATH="/usr/local"

# Source the workspace
source ~/ros2_ws/install/setup.bash
```

Add to `.bashrc`:

```bash
echo 'source ~/ros2_ws/install/setup.bash' >> ~/.bashrc
source ~/.bashrc
```

---

## Step 11 — Launch camera_ros

### Basic command

```bash
ros2 run camera_ros camera_node
```

### With explicit parameters (recommended)

The OV5647 natively outputs `NV21` (YUV420 semi-planar), which **is not compatible** with RViz or `rqt_image_view`. Use `XRGB8888` for maximum compatibility:

```bash
ros2 run camera_ros camera_node --ros-args \
  -p format:=XRGB8888 \
  -p width:=640 \
  -p height:=480
```

### OV5647 available resolutions

| Resolution | FPS | Recommended use |
|---|---|---|
| 640x480 | 58.92 fps | Real-time HRI, person detection |
| 1296x972 | 43.25 fps | Quality/performance balance ✅ |
| 1920x1080 | 30.62 fps | Recording, mapping |
| 2592x1944 | 15.63 fps | Maximum resolution, low framerate |

### Available pixel formats

| Format | Compatibility | Notes |
|---|---|---|
| `XRGB8888` | RViz ✅, rqt ✅ | Recommended for visualization |
| `YUYV` | rqt ✅ | Lower bandwidth than XRGB |
| `NV21` | ❌ RViz/rqt | Native format, requires explicit conversion |

---

## Step 12 — Verify ROS 2 publishing

```bash
# In another terminal
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash

ros2 topic list
# Should show: /image_raw  /camera_info

ros2 topic hz /image_raw
# Verify framerate

# Visualize with rqt_image_view
ros2 run rqt_image_view rqt_image_view
```

> ℹ️ The warning `Camera calibration file [...] not found` is **expected** until the camera is calibrated with `camera_calibration`. It does not block basic node operation.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| `rpicam-hello: command not found` | `rpicam-apps` not built or PATH wrong | `export PATH=$PATH:/usr/local/bin` |
| `Could not open any dmaHeap device` | `/dev/dma_heap` permissions | Complete Step 8 + reconnect SSH |
| `No cameras available` after permissions fix | Wrong `config.txt` or loose cable | Check Step 2 and physical connection |
| `libavcodec API version is too old` | FFmpeg/rpicam-apps incompatibility | Build with `-Denable_libav=disabled` |
| Two libcamera versions in `ldconfig` | Ubuntu libcamera not removed | Step 4 |
| `ninja` fails with Boost in `meson setup` | Missing `libboost-program-options-dev` | `sudo apt install libboost-program-options-dev` + `rm -rf build` |
| `fatal error: GLES2/gl2.h` | Missing OpenGL ES headers | `sudo apt install libgles2-mesa-dev` |
| `cc1plus: Killed signal` during compilation | Out of memory (OOM killer) | Create swap + use `ninja -j2` |
| `Failed to call start: -110` in camera_ros | ABI mismatch: ROS libcamera vs RPi fork | Step 10.1: remove `ros-jazzy-libcamera` + rebuild |
| `FATAL Serializer` + IPA proxy crash | Same cause as above | Same fix |
| `Unsupported image encoding [nv21]` in RViz | Native format not compatible | Launch with `-p format:=XRGB8888` |
| Camera detected over I2C but CSI timeout | Damaged cable or damaged CSI port | Try a new cable; if fails with 4+ cables → module or port damaged |
| `i2c read error = -5` in dmesg | Sensor not physically responding | Check ribbon; if persists → camera damaged |

---

## Hardware diagnostics — Module or CSI port damaged?

The OV5647 has two independent buses: **I2C** (configuration, slow signals) and **CSI MIPI** (frame transfer, high-speed). It is possible for the sensor to respond over I2C while the CSI transmitter is damaged (e.g. from ESD).

```bash
# Check I2C response from the sensor
sudo apt install i2c-tools
sudo i2cdetect -y 10
# "UU" at position 0x36 → kernel has it reserved, sensor is alive
# "--" at position 0x36 → sensor not responding = likely damaged camera
```

To isolate whether the issue is the module or the RPi's CSI port: test the same module on another RPi. If it fails on both → module damaged. If it works on the other → CSI port on the original board is damaged.

---

## Verified versions

| Component | Version |
|---|---|
| Ubuntu Server | 24.04 LTS (aarch64) |
| Raspberry Pi | 4 |
| Sensor | OV5647 (RPi Camera Module v1.3) |
| libcamera (RPi fork) | 0.7.0 |
| rpicam-apps | 1.11.1 |
| ROS 2 | Jazzy |
| camera_ros | built from source (github.com/christianrauch/camera_ros) |
| GCC | 13.3.0 |