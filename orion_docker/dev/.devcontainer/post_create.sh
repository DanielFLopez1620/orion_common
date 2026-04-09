#!/bin/bash
set -e

sudo chown -R orion:orion /home/orion/ws
echo "Importing repos..."
vcs import src < .devcontainer/repos.yaml

rosdep update
rosdep install --from-paths src --ignore-src -y