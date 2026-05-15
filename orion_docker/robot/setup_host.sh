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

echo "Adding ${USER} to the 'dialout' and 'i2c' groups..."
sudo usermod -aG dialout "${USER}"
sudo groupadd -f i2c
sudo usermod -aG i2c "${USER}"
echo "  → Log out and back in for the group changes to take effect."

echo "Enabling I2C bus (required for MPU6050)..."
if ! grep -q "^dtparam=i2c_arm=on" /boot/firmware/config.txt 2>/dev/null; then
    echo "dtparam=i2c_arm=on" | sudo tee -a /boot/firmware/config.txt
    echo "  → I2C enabled — reboot required to activate."
else
    echo "  → I2C already enabled."
fi

if ! groups "${USER}" | grep -q docker; then
    echo "Adding ${USER} to the 'docker' group..."
    sudo usermod -aG docker "${USER}"
    echo "  → Log out and back in for the group change to take effect."
fi

# Edit the CAMERA_TYPE and other args below to match your hardware setup.
SERVICE_DST="/etc/systemd/system/orion_robot.service"

echo "Installing systemd service..."
sudo tee "${SERVICE_DST}" > /dev/null << 'EOF'
[Unit]
Description=ORION Robot Bringup (Docker)
After=network-online.target docker.service
Requires=docker.service
Wants=network-online.target

[Service]
# Remove any previous container instance before starting
ExecStartPre=-/usr/bin/docker stop orion_robot
ExecStartPre=-/usr/bin/docker rm   orion_robot

# --- Edit the launch arguments to match your hardware ---
# camera options : a010 | os30a | astra_s
# ctl_type       : micro_ros | serial
ExecStart=/usr/bin/docker run \
    --rm \
    --name orion_robot \
    --privileged \
    --network host \
    -v /dev:/dev \
    -e ROS_DOMAIN_ID=0 \
    orion_robot:latest \
    ros2 launch orion_bringup bringup.launch.py camera:=a010 rasp:=rpi4 ctl_type:=micro_ros

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
