#!/bin/bash
set -e

WS_ROOT="/home/orion_user/ws"
REPOS_YAML="${WS_ROOT}/src/orion_common/orion_docker/dev/repos.yaml"

# Fix ownership only on non-bind-mounted workspace dirs (build, install, log).
# Do NOT chown src/orion_common — it is a bind mount owned by the host user.
# Chowning it would corrupt git ownership on the host (git dubious ownership error).
sudo mkdir -p "${WS_ROOT}/build" "${WS_ROOT}/install" "${WS_ROOT}/log"
sudo chown -R orion_user:orion_user \
    "${WS_ROOT}/build" \
    "${WS_ROOT}/install" \
    "${WS_ROOT}/log"

# Clone external repos (orion_common is already mounted — not listed in repos.yaml)
echo "Importing repositories into ${WS_ROOT}/src ..."
cd "${WS_ROOT}"
vcs import src < "${REPOS_YAML}"

# Install ROS dependencies for all packages.
# orion_chat declares pip-only keys that are not in the rosdep database for
# Ubuntu Noble (sounddevice, webrtcvad, python3-sounddevice, python3-soundfile,
# pytest). We skip those keys and handle them with orion_chat's own scripts.
# orion_chat MUST be included in --from-paths so that --ignore-src suppresses
# the `orion_chat` package dependency declared in orion_bringup / orion.
echo "Running rosdep..."
rosdep update
# apt-get update is required: the Docker image deletes /var/lib/apt/lists/ at
# build time, so the package index is empty when the container first starts.
# Without this, apt-get install (called internally by rosdep) cannot locate
# any package, including ros-jazzy-* ones.
sudo apt-get update
rosdep install --from-paths src --ignore-src -y \
    --skip-keys="sounddevice webrtcvad python3-sounddevice pytest"

# orion_chat: use its own install scripts instead of rosdep
ORION_CHAT_DIR="${WS_ROOT}/src/orion_chat"
if [ -d "${ORION_CHAT_DIR}" ]; then
    echo "Installing orion_chat dependencies..."
    if [ -f "${ORION_CHAT_DIR}/install_apt.sh" ]; then
        bash "${ORION_CHAT_DIR}/install_apt.sh"
    fi
    if [ -f "${ORION_CHAT_DIR}/requirements.txt" ]; then
        pip3 install -r "${ORION_CHAT_DIR}/requirements.txt" --break-system-packages
    fi
fi

echo "Done. Build the workspace with:"
echo "  cd ${WS_ROOT} && colcon build --symlink-install"
