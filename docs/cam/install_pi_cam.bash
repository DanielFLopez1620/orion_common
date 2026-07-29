#!/bin/bash
# =============================================================================
# install_pi_cam.bassh
# RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04 — Raspberry Pi 4
# For use with camera_ros in ROS 2 Jazzy
#
# USAGE:
#   First run:    sudo bash setup_rpicam_ubuntu2404.sh
#   After reboot: sudo bash setup_rpicam_ubuntu2404.sh --skip-boot
#   ROS 2 only:   bash setup_rpicam_ubuntu2404.sh --ros-only
# =============================================================================

set -e

# ─── Colors ───────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ─── Helpers ──────────────────────────────────────────────────────────────────
info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
step()    { echo -e "\n${CYAN}━━━ $1 ━━━${NC}"; }

# ─── Check environment ────────────────────────────────────────────────────────
check_environment() {
  step "Step 0/9: Checking environment"

  if [[ "$(uname -m)" != "aarch64" ]]; then
    error "This script requires aarch64 architecture (RPi 4 with 64-bit Ubuntu)"
  fi

  if ! grep -q "24.04" /etc/os-release 2>/dev/null; then
    warn "This script was tested on Ubuntu 24.04 for RPi. Continue at your own risk."
    read -rp "Continue anyway? [y/N]: " confirm
    [[ "$confirm" =~ ^[yY]$ ]] || exit 0
  fi

  success "Environment verified: aarch64 / Ubuntu 24.04"
}

# ─── Step 1: config.txt ───────────────────────────────────────────────────────
configure_boot() {
  step "Step 1/9: Configuring /boot/firmware/config.txt"

  CONFIG="/boot/firmware/config.txt"

  if [[ ! -f "$CONFIG" ]]; then
    error "$CONFIG not found. Are you on Ubuntu Server 24.04 for RPi?"
  fi

  sudo cp "$CONFIG" "${CONFIG}.bak.$(date +%Y%m%d_%H%M%S)"
  info "Backup created: ${CONFIG}.bak.*"

  if grep -q "dtoverlay=ov5647" "$CONFIG"; then
    info "dtoverlay=ov5647 already present in config.txt"
  else
    echo "" | sudo tee -a "$CONFIG" > /dev/null
    echo "# RPi Camera v1.3 (OV5647) — configured by setup_rpicam_ubuntu2404.sh" | sudo tee -a "$CONFIG" > /dev/null
    echo "camera_auto_detect=0" | sudo tee -a "$CONFIG" > /dev/null
    echo "dtoverlay=ov5647" | sudo tee -a "$CONFIG" > /dev/null
    success "config.txt updated"
  fi

  if grep -q "^camera_auto_detect=1" "$CONFIG"; then
    sudo sed -i 's/^camera_auto_detect=1/camera_auto_detect=0/' "$CONFIG"
    info "camera_auto_detect changed from 1 to 0"
  fi

  warn "A reboot is required. After rebooting, run: sudo bash $0 --skip-boot"
}

# ─── Step 2: Remove apt libcamera ─────────────────────────────────────────────
remove_apt_libcamera() {
  step "Step 2/9: Removing apt libcamera (Ubuntu upstream version, no RPi support)"

  REMOVED=0
  for pkg in libcamera-dev libcamera0.2 libcamera0.5; do
    if dpkg -l | grep -q "^ii.*$pkg"; then
      sudo apt remove --purge "$pkg" -y
      REMOVED=1
    fi
  done

  if [[ $REMOVED -eq 1 ]]; then
    sudo ldconfig
    success "apt libcamera removed"
  else
    info "apt libcamera was not installed, skipping..."
  fi
}

# ─── Step 3: Build dependencies ───────────────────────────────────────────────
install_dependencies() {
  step "Step 3/9: Installing build dependencies"

  sudo apt update
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

  success "Dependencies installed"
}

# Swap helper in case of few resources
_ensure_swap() {
  # libcamera/rpicam-apps compilation requires ~1.5 GB RAM.
  # If free memory is low, create a temporary swap file to prevent OOM kills.
  FREE_MEM=$(free -m | awk '/^Mem:/{print $7}')
  if [[ $FREE_MEM -lt 1024 ]]; then
    if [[ ! -f /swapfile ]]; then
      info "Available RAM: ${FREE_MEM}MB — creating 2GB temporary swap..."
      sudo fallocate -l 2G /swapfile
      sudo chmod 600 /swapfile
      sudo mkswap /swapfile
      sudo swapon /swapfile
      SWAP_CREATED=1
      success "Temporary swap created (2GB)"
    else
      sudo swapon /swapfile 2>/dev/null || true
      info "Existing swap activated"
    fi
  else
    info "Available RAM: ${FREE_MEM}MB — swap not needed"
  fi
}

# Delete swap after usage (if used)
_cleanup_swap() {
  if [[ "${SWAP_CREATED:-0}" -eq 1 ]]; then
    sudo swapoff /swapfile
    sudo rm /swapfile
    info "Temporary swap removed"
  fi
}

# Ensure libcamera is registered
_register_libcamera() {
  echo "/usr/local/lib/aarch64-linux-gnu" | sudo tee /etc/ld.so.conf.d/rpicam.conf > /dev/null
  sudo ldconfig
  info "Libraries registered in ldconfig"
}

# ─── Step 4: Build libcamera (RPi fork) ───────────────────────────────────────
build_libcamera() {
  step "Step 4/9: Building libcamera (Raspberry Pi fork)"

  cd ~

  if [[ -d "libcamera" ]]; then
    warn "Directory ~/libcamera already exists. Rebuild from scratch?"
    read -rp "[y/N]: " confirm
    if [[ "$confirm" =~ ^[yY]$ ]]; then
      rm -rf libcamera
    else
      info "Using existing directory, running install only..."
      cd libcamera
      sudo ninja -C build install
      _register_libcamera
      return
    fi
  fi

  _ensure_swap

  git clone https://github.com/raspberrypi/libcamera.git
  cd libcamera
  meson setup build
  ninja -C build -j2
  sudo ninja -C build install

  _cleanup_swap

  _register_libcamera
  success "libcamera (RPi fork) built and installed"
}

# ─── Step 5: Build rpicam-apps ────────────────────────────────────────────────
build_rpicam_apps() {
  step "Step 5/9: Building rpicam-apps"
  info "Note: libav encoder disabled — incompatible with Ubuntu 24.04 FFmpeg (v60.x)"

  cd ~

  if [[ -d "rpicam-apps" ]]; then
    warn "Directory ~/rpicam-apps already exists. Rebuild from scratch?"
    read -rp "[y/N]: " confirm
    if [[ "$confirm" =~ ^[yY]$ ]]; then
      rm -rf rpicam-apps
    else
      info "Using existing directory, cleaning build..."
      cd rpicam-apps && rm -rf build
    fi
  fi

  [[ ! -d ~/rpicam-apps ]] && git clone https://github.com/raspberrypi/rpicam-apps.git
  cd ~/rpicam-apps

  _ensure_swap

  meson setup build --buildtype=release -Denable_libav=disabled
  ninja -C build -j2
  sudo ninja -C build install
  sudo ldconfig

  _cleanup_swap

  success "rpicam-apps built and installed"
}

# ─── Step 6: dma_heap permissions ─────────────────────────────────────────────
configure_permissions() {
  step "Step 6/9: Configuring dma_heap permissions"

  REAL_USER="${SUDO_USER:-$USER}"

  if groups "$REAL_USER" | grep -q "video"; then
    info "User $REAL_USER already belongs to group video"
  else
    sudo usermod -aG video "$REAL_USER"
    success "User $REAL_USER added to group video"
  fi

  UDEV_FILE="/etc/udev/rules.d/99-dma-heap.rules"
  if [[ -f "$UDEV_FILE" ]]; then
    info "udev rule already exists"
  else
    echo 'SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"' | sudo tee "$UDEV_FILE" > /dev/null
    success "udev rule created"
  fi

  sudo udevadm control --reload-rules
  sudo udevadm trigger
  success "udev rules reloaded"

  warn "IMPORTANT: Log out and reconnect via SSH for the 'video' group to take effect"
}

# ─── Step 7: Verify libcamera ─────────────────────────────────────────────────
verify_libcamera() {
  step "Step 7/9: libcamera verification"

  echo ""
  info "→ V4L2 devices:"
  v4l2-ctl --list-devices 2>/dev/null || warn "v4l2-ctl found no devices"

  echo ""
  info "→ Cameras detected by libcamera:"
  rpicam-hello --list-cameras 2>/dev/null || warn "Could not list cameras. Reconnect SSH session if you see 'dmaHeap'."

  echo ""
  success "libcamera verification complete"
}

# ─── Step 8: Set up camera_ros for ROS 2 ──────────────────────────────────────
setup_ros2_camera() {
  step "Step 8/9: Setting up camera_ros for ROS 2 Jazzy"

  if [[ ! -f "/opt/ros/jazzy/setup.bash" ]]; then
    error "ROS 2 Jazzy not found at /opt/ros/jazzy. Please install it first."
  fi

  # 8.1 — Remove ros-jazzy-libcamera (ABI mismatch with RPi fork)
  info "Removing ros-jazzy-libcamera to prevent ABI conflict with RPi fork..."
  info "This package causes: 'FATAL Serializer' and 'Failed to call start: -110'"

  for pkg in ros-jazzy-libcamera ros-jazzy-camera-ros; do
    if dpkg -l | grep -q "^ii.*$pkg"; then
      sudo apt remove --purge "$pkg" -y
      info "Removed: $pkg"
    fi
  done
  sudo ldconfig

  if ls /opt/ros/jazzy/lib/libcamera.so* 2>/dev/null | grep -q .; then
    warn "libcamera.so still present in /opt/ros/jazzy/lib/ — conflicts may occur"
  else
    success "libcamera conflict resolved"
  fi

  # 8.2 — Install camera-info and ROS dependencies
  info "Installing camera-info and ROS image transport dependencies..."
  sudo apt install -y \
    ros-jazzy-camera-info-manager \
    ros-jazzy-image-transport \
    ros-jazzy-image-transport-plugins
  success "ROS dependencies installed"

  # 8.3 — Set environment variables
  info "Configuring environment variables for RPi fork libcamera..."

  REAL_USER="${SUDO_USER:-$USER}"
  REAL_HOME=$(getent passwd "$REAL_USER" | cut -d: -f6)
  BASHRC="$REAL_HOME/.bashrc"

  ENV_BLOCK='
# libcamera RPi fork — prevent conflicts with apt/ROS version
export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera'

  if ! grep -q "LIBCAMERA_IPA_MODULE_PATH" "$BASHRC"; then
    echo "$ENV_BLOCK" >> "$BASHRC"
    success "Environment variables added to $BASHRC"
  else
    info "Environment variables already present in $BASHRC"
  fi

  export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
  export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
  export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera

  # 8.4 — Build camera_ros from source
  info "Building camera_ros from source (will link against RPi fork of libcamera)..."

  WS="$REAL_HOME/picam_ws"
  mkdir -p "$WS/src"
  cd "$WS"

  if [[ -d "src/camera_ros" ]]; then
    warn "src/camera_ros already exists. Rebuild from scratch?"
    read -rp "[y/N]: " confirm
    if [[ "$confirm" =~ ^[yY]$ ]]; then
      rm -rf src/camera_ros build/camera_ros install/camera_ros
      sudo -u "$REAL_USER" git clone https://github.com/christianrauch/camera_ros.git src/camera_ros
    fi
  else
    sudo -u "$REAL_USER" git clone https://github.com/christianrauch/camera_ros.git src/camera_ros
  fi

  sudo -u "$REAL_USER" bash -c "
    source /opt/ros/jazzy/setup.bash
    export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:\$PKG_CONFIG_PATH
    export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
    export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera
    cd $WS
    colcon build --packages-select camera_ros \
      --cmake-args -DCMAKE_PREFIX_PATH='/usr/local'
  "

  if ! grep -q "picam_ws/install/setup.bash" "$BASHRC"; then
    echo "source $WS/install/setup.bash" >> "$BASHRC"
    success "Workspace sourced in $BASHRC"
  fi

  success "camera_ros built at $WS"
}

# ─── Step 9: Final ROS 2 verification ─────────────────────────────────────────
verify_ros2() {
  step "Step 9/9: Final verification"

  source /opt/ros/jazzy/setup.bash

  REAL_USER="${SUDO_USER:-$USER}"
  REAL_HOME=$(getent passwd "$REAL_USER" | cut -d: -f6)
  source "$REAL_HOME/picam_ws/install/setup.bash" 2>/dev/null || true

  echo ""
  info "→ Checking camera_ros package:"
  ros2 pkg list | grep camera_ros || warn "camera_ros not found in ros2 pkg list"

  echo ""
  success "━━━ Setup complete ━━━"

  # Log usage
  echo ""
  echo -e "${CYAN}Recommended launch command for OV5647:${NC}"
  echo ""
  echo "  ros2 run camera_ros camera_node --ros-args \\"
  echo "    -p format:=YUYV \\"
  echo "    -p width:=640 \\"
  echo "    -p height:=480"
  echo ""
  echo -e "${CYAN}OV5647 available resolutions:${NC}"
  echo "  320x240   @ >60.0 fps  → constrained env"
  echo "  640x480   @ 58.92 fps  → real-time HRI"
  echo "  1296x972  @ 43.25 fps  → quality/performance balance"
  echo "  1920x1080 @ 30.62 fps  → recording / mapping"
  echo "  2592x1944 @ 15.63 fps  → maximum resolution"
  echo ""
  echo -e "${CYAN}Verify publishing:${NC}"
  echo "  ros2 topic list          # should show /image_raw and /camera_info"
  echo "  ros2 topic hz /image_raw"
  echo ""
  warn "If you see 'Unsupported image encoding [nv21]' in RViz → use -p format:=XRGB8888"
  warn "The 'calibration file not found' warning is expected until you calibrate the camera"
}

# ─── Main ─────────────────────────────────────────────────────────────────────
main() {
  echo -e "${CYAN}"
  echo "╔══════════════════════════════════════════════════════════╗"
  echo "║   RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04        ║"
  echo "║   Full setup with camera_ros in ROS 2 Jazzy              ║"
  echo "╚══════════════════════════════════════════════════════════╝"
  echo -e "${NC}"

  case "$1" in
    --skip-boot)
      info "Post-reboot mode: skipping boot configuration"
      remove_apt_libcamera
      install_dependencies
      build_libcamera
      build_rpicam_apps
      configure_permissions
      verify_libcamera
      setup_ros2_camera
      verify_ros2
      echo ""
      warn "Log out and reconnect via SSH for the 'video' group to take effect."
      warn "Then launch: ros2 run camera_ros camera_node --ros-args -p format:=XRGB8888 -p width:=640 -p height:=480"
      ;;

    --ros-only)
      info "ROS-only mode: setting up camera_ros only (libcamera already installed)"
      setup_ros2_camera
      verify_ros2
      ;;

    *)
      check_environment
      configure_boot

      echo ""
      echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
      echo -e "${YELLOW}  A REBOOT is required to apply config.txt changes            ${NC}"
      echo -e "${YELLOW}  After rebooting, run:                                       ${NC}"
      echo -e "${YELLOW}    sudo bash $(realpath $0) --skip-boot                      ${NC}"
      echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
      echo ""

      read -rp "Reboot now? [y/N]: " reboot_now
      if [[ "$reboot_now" =~ ^[yY]$ ]]; then
        sudo reboot
      fi
      ;;
  esac
}

main "$@"