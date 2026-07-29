#!/bin/bash
# setup_host.sh — run ONCE on the Raspberry Pi host before using orion_robot container.
#
# This script installs the host-level prerequisites that cannot live inside Docker:
#   1. udev rules for robot peripherals
#   2. Current user added to 'dialout' and 'i2c' groups
#   3. I2C bus enabled in /boot/firmware/config.txt (for MPU6050)
#   4. systemd service that starts the container on boot
#
# Run from the root of the orion_common repository:
#   bash orion_docker/robot/setup_host.sh
set -e

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_DIR="${REPO_ROOT}/orion_docker/robot"

# ── Sanity checks ─────────────────────────────────────────────────────────────
if [ "$(id -u)" -eq 0 ]; then
    echo "ERROR: Do not run this script as root. Run it as your regular user."
    exit 1
fi

if ! command -v docker &>/dev/null; then
    echo "ERROR: Docker is not installed. Install it first:"
    echo "  https://docs.docker.com/engine/install/ubuntu/"
    exit 1
fi

# Do not forget to validate that the udev rules apply to your devices
UDEV_SRC="${REPO_ROOT}/orion_bringup/example_udev.rules"
UDEV_DST="/etc/udev/rules.d/99-orion.rules"

echo "Installing udev rules..."
sudo cp "${UDEV_SRC}" "${UDEV_DST}"
sudo udevadm control --reload-rules
sudo udevadm trigger
echo "  → ${UDEV_DST}"
echo "  ⚠  Review and update the rules to match your device attributes:"
echo "     sudo nano ${UDEV_DST}"

echo "Adding ${USER} to the 'dialout', 'i2c', 'pwm', and 'video' groups..."
sudo usermod -aG dialout "${USER}"
sudo groupadd -f i2c
sudo usermod -aG i2c "${USER}"
sudo groupadd -f pwm
sudo usermod -aG pwm "${USER}"
# 'video' is required for the dma_heap udev rule (picam / libcamera).
sudo usermod -aG video "${USER}"
echo "  → Log out and back in for the group changes to take effect."

echo "Enabling I2C bus (required for MPU6050)..."
if ! grep -q "^dtparam=i2c_arm=on" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtparam=i2c_arm=on" | sudo tee -a /boot/firmware/config.txt
    echo "  → I2C enabled — reboot required to activate."
else
    echo "  → I2C already enabled."
fi

echo "Configuring OV5647 picam overlay (required when launching with g_mov:=true)..."
if ! grep -q "^camera_auto_detect=0" /boot/firmware/config.txt 2>/dev/null; then
    echo "camera_auto_detect=0" | sudo tee -a /boot/firmware/config.txt
    echo "  → camera_auto_detect=0 added — reboot required to activate."
else
    echo "  → camera_auto_detect already disabled."
fi
if ! grep -q "^dtoverlay=ov5647" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtoverlay=ov5647" | sudo tee -a /boot/firmware/config.txt
    echo "  → dtoverlay=ov5647 added — reboot required to activate."
else
    echo "  → dtoverlay=ov5647 already configured."
fi

echo "Installing dma_heap udev rule (libcamera access for non-root users)..."
if [ ! -f /etc/udev/rules.d/99-dma-heap.rules ]; then
    echo 'SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"' | \
        sudo tee /etc/udev/rules.d/99-dma-heap.rules > /dev/null
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    echo "  → /etc/udev/rules.d/99-dma-heap.rules"
else
    echo "  → dma_heap rule already present."
fi

echo "Checking picam prerequisites (libcamera RPi fork + camera_ros)..."
if [ ! -f /usr/local/lib/aarch64-linux-gnu/libcamera.so ]; then
    echo "  ⚠  libcamera (RPi fork) is NOT installed in /usr/local."
    echo "     Follow docs/cam/README.md (Steps 4–9) to build and install it"
    echo "     before launching with g_mov:=true."
fi
if [ ! -f "${HOME}/picam_ws/install/setup.bash" ]; then
    echo "  ⚠  camera_ros workspace not found at ${HOME}/picam_ws."
    echo "     Follow docs/cam/README.md (Step 10) to build it. The systemd"
    echo "     service below bind-mounts this path into the container."
fi

echo "Enabling hardware PWM on GPIO12 (required for MG996R servo)..."
if ! grep -q "^dtoverlay=pwm" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtoverlay=pwm,gpiopin=12,func=4" | sudo tee -a /boot/firmware/config.txt
    echo "  → dtoverlay=pwm,gpiopin=12,func=4 added — reboot required to activate."
else
    echo "  → Hardware PWM (dtoverlay) already configured."
fi

echo "Installing pwm-setup.service (exports PWM channel before container start)..."
sudo tee /etc/systemd/system/pwm-setup.service > /dev/null << 'EOF'
[Unit]
Description=Export and unlock PWM0 for ORION servo node
After=local-fs.target
Before=orion_robot.service

[Service]
Type=oneshot
ExecStart=/bin/sh -c 'echo 0 > /sys/class/pwm/pwmchip0/export; chmod -R a+rw /sys/class/pwm/pwmchip0/'
RemainAfterExit=yes

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable pwm-setup.service
echo "  → /etc/systemd/system/pwm-setup.service (enabled)"

if ! groups "${USER}" | grep -q docker; then
    echo "Adding ${USER} to the 'docker' group..."
    sudo usermod -aG docker "${USER}"
    echo "  → Log out and back in for the group change to take effect."
fi

# Edit the CAMERA_TYPE and other args below to match your hardware setup.
SERVICE_DST="/etc/systemd/system/orion_robot.service"

echo "Installing systemd service..."
sudo tee "${SERVICE_DST}" > /dev/null << EOF
[Unit]
Description=ORION Robot Bringup (Docker)
After=network-online.target docker.service pwm-setup.service
Requires=docker.service pwm-setup.service
Wants=network-online.target

[Service]
# Remove any previous container instance before starting
ExecStartPre=-/usr/bin/docker stop orion_robot
ExecStartPre=-/usr/bin/docker rm   orion_robot

# --- Edit the launch arguments to match your hardware ---
# camera options : a010 | os30a | astra_s
# ctl_type       : micro_ros | serial
# g_mov          : true | false  (enables picam + IMU + pan/tilt servo)
#
# The mounts and LIBCAMERA_* env vars are only used when g_mov:=true and
# expose the host's libcamera (RPi fork) + camera_ros build to the container.
# They are harmless when g_mov:=false.
ExecStart=/usr/bin/docker run \\
    --rm \\
    --name orion_robot \\
    --privileged \\
    --ulimit rtprio=99 \\
    --ulimit memlock=-1 \\
    --network host \\
    -v /dev:/dev \\
    -v /usr/local:/usr/local:ro \\
    -v ${HOME}/picam_ws:/home/orion_user/picam_ws:ro \\
    -e ROS_DOMAIN_ID=0 \\
    -e LD_LIBRARY_PATH=/usr/local/lib/aarch64-linux-gnu \\
    -e LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera \\
    -e LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera \\
    orion_robot:latest \\
    ros2 launch orion_bringup bringup.launch.py camera:=a010 rasp:=rpi4 g_mov:=true ctl_type:=micro_ros

Restart=on-failure
StandardOutput=journal
StandardError=journal

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable orion_robot.service
echo "  → ${SERVICE_DST} (enabled, will start on next boot)"
echo ""
echo "To start it now without rebooting:"
echo "  sudo systemctl start orion_robot.service"
echo ""
echo "To watch the logs:"
echo "  journalctl -u orion_robot.service -f"
echo ""
echo "Setup complete. Reboot or log out/in for group changes to take effect."
