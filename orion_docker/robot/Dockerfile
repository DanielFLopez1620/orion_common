FROM orion_base:latest

ARG USERNAME=orion_user
ARG USER_UID=1001

USER root

# ── 1. Clone, build and install workspace into the image ─────────────────────
# Unlike orion_dev, there is no bind mount at runtime — packages are compiled
# directly into the image for stable, reproducible deployments on the robot.
# src/ and build/ are removed after colcon build to reduce image size.
# Single RUN required to chain source calls across steps.
COPY repos.yaml /tmp/robot_repos.yaml

RUN /bin/bash -c " \
    set -e && \
    mkdir -p /home/${USERNAME}/ws/src && \
    cd /home/${USERNAME}/ws && \
    source /opt/ros/${ROS_DISTRO}/setup.bash && \
    vcs import src < /tmp/robot_repos.yaml && \
    apt-get update && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -y && \
    rm -rf /var/lib/apt/lists/* && \
    colcon build --cmake-args -DCMAKE_BUILD_TYPE=Release && \
    rm -rf src build log && \
    chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/ws \
    "

# ── 2. Entrypoint ─────────────────────────────────────────────────────────────
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

# ── 3. User shell setup ───────────────────────────────────────────────────────
USER $USERNAME

RUN echo "source /home/${USERNAME}/ws/install/setup.bash" >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

# Default launch — override at runtime with:
#   docker run orion_robot:latest ros2 launch orion_bringup bringup.launch.py camera:=os30a
ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "launch", "orion_bringup", "bringup.launch.py"]
