#!/bin/bash
set -e

WS_ROOT="/home/orion_user/ws"
REPOS_YAML="${WS_ROOT}/src/orion_common/orion_docker/dev/repos.yaml"

# Fix ownership only on non-bind-mounted workspace dirs (build, install, log).
sudo mkdir -p "${WS_ROOT}/build" "${WS_ROOT}/install" "${WS_ROOT}/log"
sudo chown -R orion_user:orion_user \
    "${WS_ROOT}/build" \
    "${WS_ROOT}/install" \
    "${WS_ROOT}/log"

echo "Importing repositories into ${WS_ROOT}/src ..."
cd "${WS_ROOT}"
vcs import src < "${REPOS_YAML}"

echo "Running rosdep..."
rosdep update
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
