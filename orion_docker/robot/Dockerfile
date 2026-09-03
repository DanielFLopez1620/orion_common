FROM orion_base:latest

ARG USERNAME=orion_user
ARG USER_UID=1001

USER root

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-nav2-lifecycle-manager \
    ros-${ROS_DISTRO}-laser-filters \
    ros-${ROS_DISTRO}-v4l2-camera \
    ros-${ROS_DISTRO}-joy \
    ros-${ROS_DISTRO}-teleop-twist-joy \
    libudev-dev \
    libgtk-3-dev \
    libdc1394-dev \
    && rm -rf /var/lib/apt/lists/*

RUN MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)" \
    && ln -s "/usr/lib/${MULTIARCH}/libdc1394.so.25" \
             "/usr/lib/${MULTIARCH}/libdc1394.so.22"

COPY repos.yaml /tmp/robot_repos.yaml

RUN /bin/bash -c " \
    set -e && \
    mkdir -p /home/${USERNAME}/ws/src && \
    cd /home/${USERNAME}/ws && \
    source /opt/ros/${ROS_DISTRO}/setup.bash && \
    vcs import src < /tmp/robot_repos.yaml && \
    git -C src/ldrobot_lidar_ros2 submodule update --init --recursive && \
    apt-get update && \
    rosdep update && \
    rosdep install --from-paths src --ignore-src -y \
        --skip-keys pytest \
        --skip-keys python3-pytest && \
    rm -rf /var/lib/apt/lists/* && \
    MAKEFLAGS=\"-j1\" colcon build \
        --parallel-workers 1 \
        --cmake-args -DCMAKE_BUILD_TYPE=Release && \
    find build/ldlidar_component -name \"libldlidar.so\" \
        -exec cp {} install/ldlidar_component/lib/ \; && \
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
