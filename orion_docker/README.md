# ORION Docker

Containerized environments for developing and deploying ORION robot on ROS 2 Jazzy.

---

## Architecture

The containers follow a parent–child hierarchy to avoid duplicating dependencies:

```plaintext
osrf/ros:jazzy-ros-base
        │
        ▼
  orion_base          ← common deps: ros2_control, micro-ROS agent, CycloneDDS,
     /      \            camera stack, peripheral libs, build tools
    ▼         ▼
orion_dev   orion_robot
(PC)        (Raspberry Pi — build natively on the RPi)
```

| Image | Purpose | Built on |
| --- | --- | --- |
| `orion_base` | Shared foundation: ROS 2 tools, ros2_control, micro-ROS agent, sensor driver deps | Developer PC |
| `orion_dev` | Full development environment: Gazebo, RViz2, Nav2, SLAM, ORBBEC deps, OpenCV. Managed as a VS Code devcontainer. | Developer PC |
| `orion_robot` | Minimal runtime for the robot hardware. No GUI, no simulation. | Raspberry Pi (native build) |

---

## Prerequisites

- [Docker Engine](https://docs.docker.com/engine/install/ubuntu/)
- [VS Code](https://code.visualstudio.com/) with the [Dev Containers](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) and [Docker](https://marketplace.visualstudio.com/items?itemName=ms-azuretools.vscode-docker) extensions

---

## 1. Build `orion_base`

The base image must be built before `orion_dev` or `orion_robot`.

```bash
git clone https://github.com/DanielFLopez1620/orion_common.git
cd orion_common

docker build -t orion_base:latest orion_docker/base/
```

> This step takes ~15 minutes on first build — the micro-ROS agent is compiled from source.

---

## 2. Development container (`orion_dev`)

### Host setup (run once on the developer PC)

#### X11 display (RViz2, Gazebo)

GUI applications require X11 forwarding. Run this on the host before launching any GUI tool inside the container:

```bash
xhost +local:docker
```

To apply this automatically on login:

```bash
echo "xhost +local:docker" >> ~/.profile
```

#### ORBBEC Astra S udev rules

Required for the camera to be accessible from inside the container. The udev rules script lives inside `depth_orbbec_astra`, which is cloned by `post_create.sh` when the container first starts. Run this **after** the container has run `post_create.sh` at least once:

```bash
# On the host (not inside the container), from the workspace root
bash ~/ws/src/depth_orbbec_astra/orbbec_camera/scripts/install_udev_rules.sh
sudo udevadm control --reload-rules && sudo udevadm trigger
```

### Build

```bash
docker build -t orion_dev:latest orion_docker/dev/
```

> First build takes ~10 minutes — installs Nav2, SLAM Toolbox, Cartographer, and the simulation stack.

### Open in VS Code

The repository includes a `.devcontainer/` configuration at its root (symlink to `orion_docker/dev/`). Open the `orion_common` folder in VS Code and use:

```plaintext
Ctrl + Shift + P → Dev Containers: Reopen in Container
```

VS Code will start the container and automatically run `orion_docker/dev/post_create.sh`, which:

- Clones the external repositories (`orion_gz`, `orion_tools`, sensor drivers) into `ws/src/`
- Runs `rosdep install` for any remaining dependencies

> **Important:** Open the `orion_common` root directory in VS Code — not a subdirectory.

### Build the ROS 2 workspace

Once the container is running and `post_create.sh` has finished:

```bash
cd ~/ws
colcon build --symlink-install --cmake-args -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
source install/setup.bash
```

The `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` flag generates `compile_commands.json`, which enables full C++ IntelliSense in VS Code.

To build while skipping a specific package:

```bash
colcon build --symlink-install --packages-ignore depth_ydlidar_os30a orbbec_camera
```

### GPU support

`--gpus all` is disabled by default in `devcontainer.json`. To enable it, install the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html) on the host and uncomment the flag in `orion_docker/dev/devcontainer.json`.

---

## 3. Robot container (`orion_robot`)

> **Note:** Build this image directly on the Raspberry Pi — it targets `linux/arm64`.

Unlike `orion_dev`, the robot image **compiles the workspace inside the image** at build time. There is no bind mount at runtime — the container is self-contained and launched by a systemd service on boot.

### Host setup (run once on the RPi)

Before building or running the container, prepare the host with:

```bash
git clone https://github.com/DanielFLopez1620/orion_common.git
cd orion_common

bash orion_docker/robot/setup_host.sh
```

This script:

- Installs udev rules for all robot peripherals (`/etc/udev/rules.d/99-orion.rules`)
- Adds the current user to the `dialout` and `docker` groups
- Installs and enables the `orion_robot.service` systemd unit

> **Important:** After running the script, edit the udev rules to match the actual
> serial/ID_PATH attributes of **your** specific devices:
>
> ```bash
> sudo nano /etc/udev/rules.d/99-orion.rules
> ```
>
> Refer to [orion_bringup/README.md](../orion_bringup/README.md) for guidance on reading device attributes.

### Build the robot image

Before building, verify that all peripheral device symlinks exist on the host — the workspace build does not require them, but it confirms udev rules are correctly applied:

```bash
# Validate udev rules for ESP32s, LIDAR and your camera
ls -la /dev/ttyESP32_1 /dev/ttyESP32_2 /dev/ttyLD19 /dev/ttyA010
```

```bash
# Build base first (if not already done)
docker build -t orion_base:latest orion_docker/base/

# Build robot image (~30–60 min on RPi4 — full workspace compiled from source)
docker build -t orion_robot:latest orion_docker/robot/
```

> The workspace is built with `--parallel-workers 1` and `MAKEFLAGS="-j1"` (one package at a time, one compiler thread per package). This is the safe default for RPi4 with 2–4 GB RAM. On RPi4 with 4 GB you can raise `--parallel-workers` to `2` in `robot/Dockerfile` to cut build time roughly in half. Monitor memory during the build with `watch -n5 free -h`. If the build is killed by the OOM killer, add swap before retrying: `sudo fallocate -l 2G /swapfile && sudo chmod 600 /swapfile && sudo mkswap /swapfile && sudo swapon /swapfile`.

### Run

> **Important:** All peripheral devices (ESP32s, LIDAR, camera) must be **connected and recognized by the host before starting the container**. Docker mounts `/dev` at container start time — devices plugged in afterwards are not visible inside the container.
>
> Verify before starting:
>
> ```bash
> ls -la /dev/ttyESP32_1 /dev/ttyESP32_2 /dev/ttyLD19
> ```

The systemd service starts the container automatically on boot. To control it manually:

```bash
# Start
sudo systemctl start orion_robot.service

# Stop
sudo systemctl stop orion_robot.service

# Follow logs
journalctl -u orion_robot.service -f
```

To run manually (e.g. to test or override launch arguments):

```bash
docker run --rm --privileged --network host \
    -e ROS_DOMAIN_ID=0 \
    orion_robot:latest \
    ros2 launch orion_bringup bringup.launch.py camera:=a010 ctl_type:=micro-ros
```

### Packages included

| Package | Source |
| --- | --- |
| `orion_common` (description, control, bringup) | `main` branch |
| `depth_maixsense_a010` | `main` branch |
| `depth_ydlidar_os30a` | `main` branch |

`depth_orbbec_astra` is intentionally excluded — compilation runs out of memory on RPi4.

---

## Directory structure

```plaintext
orion_docker/
├── base/
│   ├── Dockerfile          ← shared base image
│   └── .dockerignore
├── dev/
│   ├── Dockerfile          ← development image (inherits base)
│   ├── devcontainer.json   ← VS Code devcontainer config
│   ├── repos.yaml          ← external repos cloned at container start
│   └── post_create.sh      ← workspace setup script
└── robot/
    ├── Dockerfile          ← robot deployment image (inherits base)
    ├── repos.yaml          ← packages built into the image
    ├── entrypoint.sh       ← sources ROS + micro-ROS + workspace, then exec CMD
    └── setup_host.sh       ← one-time RPi host setup (udev, groups, systemd)
```

---

## Dependency notes

### depth_ydlidar_os30a (eYs3D / libdc1394)

The os30a package ships a prebuilt `libeSPDI` library compiled against `libdc1394.so.22` (the Ubuntu 20/22.04 soname). Ubuntu 24.04 Noble ships `libdc1394.so.25` — same API, renamed soname. `orion_dev/Dockerfile` creates a compatibility symlink:

```plaintext
/usr/lib/x86_64-linux-gnu/libdc1394.so.22 → libdc1394.so.25
```

No manual steps required — it is baked into the image.

### depth_orbbec_astra (udev rules on dev PC)

The ORBBEC camera requires udev rules to be installed on the **host machine** (not inside the container). Run once after cloning:

```bash
# Run on the host after post_create.sh has cloned the repos
bash ~/ws/src/depth_orbbec_astra/orbbec_camera/scripts/install_udev_rules.sh
sudo udevadm control --reload-rules && sudo udevadm trigger
```

---

## Troubleshooting

### **`Authorization required` when launching RViz2 or Gazebo**

Run `xhost +local:docker` on the host. See [X11 display](#x11-display-rviz2-gazebo) above.

### **`fatal: detected dubious ownership` in git**

The container changed file ownership on the bind-mounted directory. Fix with:

```bash
sudo chown -R $USER:$USER /path/to/orion_common
```

### **`failed to discover GPU vendor from CDI`**

The `--gpus all` flag requires NVIDIA Container Toolkit with CDI configured. Remove or comment out `"--gpus", "all"` in `devcontainer.json` to run without GPU passthrough.

### **`libdc1394.so.22: not found` when building depth_ydlidar_os30a**

The ABI symlink is missing. This is normally baked into `orion_dev`. If building outside the devcontainer, run:

```bash
sudo ln -s /usr/lib/x86_64-linux-gnu/libdc1394.so.25 \
           /usr/lib/x86_64-linux-gnu/libdc1394.so.22
```

### **micro-ROS agent not found**

The agent is installed at `/opt/microros_ws/install/`. For interactive shells, verify it is sourced in `.bashrc`:

```bash
source /opt/microros_ws/install/setup.bash
```

For the robot container, the `entrypoint.sh` is what sources the environment (not `.bashrc` — `.bashrc` is only for interactive shells). Verify `entrypoint.sh` sources micro-ROS:

```bash
grep microros orion_docker/robot/entrypoint.sh
# expected: source /opt/microros_ws/install/setup.bash
```

### **Problems related with ORION robot**

In case of issues related to the robot, please go to the proper directory of the problem, for example, for ESP32's issues go to the [orion_base/README.md](/orion_base/README.md)
