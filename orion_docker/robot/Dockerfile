FROM orion_base:latest

ARG USERNAME=orion_user
ARG USER_UID=1001

USER root

COPY repos.yaml /tmp/robot_repos.yaml

RUN /bin/bash -c " \
    set -e && \
    mkdir -p /home/${USERNAME}/ws/src && \
    cd /home/${USERNAME}/ws && \
    source /opt/ros/${ROS_DISTRO}/setup.bash && \
    vcs import src < /tmp/robot_repos.yaml && \
    apt-get update && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -y \
        --skip-keys sounddevice webrtcvad python3-sounddevice pytest python3-pytest python3-mediapipe && \
    rm -rf /var/lib/apt/lists/* && \
    colcon build \
        --parallel-workers 2 \
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
