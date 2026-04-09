FROM ros:jazzy-ros-base

ARG USERNAME=orion_user
# 1001 avoids collision with 'ubuntu' (UID/GID 1000) already in osrf/ros:jazzy-ros-base.
# Override at build time with: --build-arg USER_UID=$(id -u)
ARG USER_UID=1001
ARG USER_GID=$USER_UID

# ── 1. User setup ─────────────────────────────────────────────────────────────
# Create group + user, grant passwordless sudo, add to hardware groups.
# groupadd || true: tolerates GID collisions with the upstream base image.
RUN groupadd --gid $USER_GID $USERNAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y --no-install-recommends sudo \
    && echo "$USERNAME ALL=(root) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && usermod -aG dialout,video,audio $USERNAME \
    && rm -rf /var/lib/apt/lists/*

# ── 2. System deps + ROS 2 packages ──────────────────────────────────────────
# Single layer: system tools, build tools, peripheral libs, ROS 2 packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    # ---- System tools
    git \
    cmake \
    curl \
    wget \
    apt-utils \
    bash-completion \
    python3-argcomplete \
    # ---- Python build tools
    python3-pip \
    python3-setuptools \
    python3-vcstool \
    python3-rosdep \
    python3-colcon-common-extensions \
    python3-colcon-mixin \
    # ---- Serial / USB  (ESP32 serial bridge, Maixsense A010)
    python3-serial \
    # ---- Peripheral system libs
    # YDLIDAR OS30A requires libdc1394
    libdc1394-dev \
    # backward-ros (debug stack traces) requires libdw
    libdw-dev \
    # ---- ROS 2 middleware
    ros-${ROS_DISTRO}-rmw-cyclonedds-cpp \
    # ---- ros2_control stack
    ros-${ROS_DISTRO}-ros2-control \
    ros-${ROS_DISTRO}-ros2-controllers \
    # ---- Camera common stack (needed by maixsense, ydlidar and orbbec drivers)
    ros-${ROS_DISTRO}-image-transport \
    ros-${ROS_DISTRO}-image-transport-plugins \
    ros-${ROS_DISTRO}-compressed-image-transport \
    ros-${ROS_DISTRO}-image-publisher \
    ros-${ROS_DISTRO}-camera-info-manager \
    # ---- Diagnostics (used by camera and sensor drivers)
    ros-${ROS_DISTRO}-diagnostic-updater \
    ros-${ROS_DISTRO}-diagnostic-msgs \
    ros-${ROS_DISTRO}-statistics-msgs \
    ros-${ROS_DISTRO}-backward-ros \
    && rm -rf /var/lib/apt/lists/*

# ── 3. colcon mixin ───────────────────────────────────────────────────────────
# || true: ros:jazzy-ros-base already ships 'default' mixin — add fails if it exists
RUN colcon mixin add default \
        https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml \
    || true \
    && colcon mixin update default

# ── 4. rosdep ─────────────────────────────────────────────────────────────────
# || true: rosdep init fails (non-zero) if already initialized — safe to ignore
RUN rosdep init || true \
    && rosdep update

# ── 5. micro-ROS agent (not available as apt package for Jazzy) ──────────────
# Built from source into /opt/microros_ws — inherited by both orion_robot and orion_dev.
# Single RUN required: each Docker RUN is a fresh shell, so source calls cannot
# span layers. Cleanup of src/build/log keeps the layer size minimal.
RUN /bin/bash -c " \
    set -e && \
    mkdir -p /opt/microros_ws/src && \
    cd /opt/microros_ws && \
    git clone -b ${ROS_DISTRO} \
        https://github.com/micro-ROS/micro_ros_setup.git \
        src/micro_ros_setup && \
    source /opt/ros/${ROS_DISTRO}/setup.bash && \
    apt-get update && \
    rosdep install --from-paths src --ignore-src -y && \
    rm -rf /var/lib/apt/lists/* && \
    colcon build && \
    source install/local_setup.bash && \
    ros2 run micro_ros_setup create_agent_ws.sh && \
    ros2 run micro_ros_setup build_agent.sh && \
    rm -rf src build log agent_ws/src agent_ws/build agent_ws/log \
    "

# ── 6. Environment ───────────────────────────────────────────────────────────
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp
ENV SHELL=/bin/bash

# ── 7. User shell setup ───────────────────────────────────────────────────────
USER $USERNAME

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /home/${USERNAME}/.bashrc \
    && echo "source /opt/microros_ws/agent_ws/install/setup.bash" >> /home/${USERNAME}/.bashrc \
    && echo "export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp" >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

CMD ["/bin/bash"]
