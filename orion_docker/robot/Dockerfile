FROM orion_base:latest

ARG USERNAME=orion_user
ARG USER_UID=1001

USER root

COPY repos.yaml /tmp/robot_repos.yaml

# ── 1. Runtime deps not in orion_base ────────────────────────────────────────
# These packages run ON the robot. Navigation, SLAM, and visualization
# run on the external PC and are intentionally excluded here.
RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-nav2-lifecycle-manager \
    ros-${ROS_DISTRO}-laser-filters \
    ros-${ROS_DISTRO}-v4l2-camera \
    ros-${ROS_DISTRO}-joy \
    ros-${ROS_DISTRO}-teleop-twist-joy \
    && rm -rf /var/lib/apt/lists/*

RUN /bin/bash -c " \
    set -e && \
    mkdir -p /home/${USERNAME}/ws/src && \
    cd /home/${USERNAME}/ws && \
    source /opt/ros/${ROS_DISTRO}/setup.bash && \
    vcs import src < /tmp/robot_repos.yaml && \
    apt-get update && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -y \
        --skip-keys sounddevice \
        --skip-keys webrtcvad  \
        --skip-keys python3-sounddevice \
        --skip-keys pytest \
        --skip-keys python3-pytest \
        --skip-keys python3-mediapipe && \
    rm -rf /var/lib/apt/lists/* && \
    MAKEFLAGS="-j1" colcon build \
        --parallel-workers 1 \
        --cmake-args -DCMAKE_BUILD_TYPE=Release && \
    rm -rf src build log && \
    chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/ws \
    "

COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

USER $USERNAME

RUN echo "source /home/${USERNAME}/ws/install/setup.bash" >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

ENTRYPOINT ["/entrypoint.sh"]
CMD ["ros2", "launch", "orion_bringup", "bringup.launch.py"]
