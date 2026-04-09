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

# Install ROS dependencies for all packages in the workspace
echo "Running rosdep..."
rosdep update
rosdep install --from-paths src --ignore-src -y

echo "Done. Build the workspace with:"
echo "  cd ${WS_ROOT} && colcon build --symlink-install"
