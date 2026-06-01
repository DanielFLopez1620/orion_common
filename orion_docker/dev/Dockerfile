FROM orion_base:latest

# Match the user defined in base — override with --build-arg if needed
ARG USERNAME=orion_user
ARG USER_UID=1001

# Switch to root for system-level installations
USER root

# Does NOT repeat what orion_base already installs.
# ALL known workspace dependencies are baked in here so that post_create.sh
# only needs to clone repos — not run heavy apt installs at container start.
RUN apt-get update && apt-get install -y --no-install-recommends \
    # ---- Simulation + visualization (PC only — too heavy for RPi)
    ros-${ROS_DISTRO}-ros-gz \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    ros-${ROS_DISTRO}-gz-ros2-control \
    # ---- Navigation stack (nav2 meta-package pulls all nav2-* components)
    ros-${ROS_DISTRO}-navigation2 \
    ros-${ROS_DISTRO}-nav2-bringup \
    # ---- SLAM / mapping
    ros-${ROS_DISTRO}-slam-toolbox \
    ros-${ROS_DISTRO}-cartographer-ros \
    ros-${ROS_DISTRO}-cartographer-ros-msgs \
    # ---- Joystick / teleoperation
    ros-${ROS_DISTRO}-joy \
    ros-${ROS_DISTRO}-teleop-twist-joy \
    # ---- Camera drivers
    ros-${ROS_DISTRO}-v4l2-camera \
    # ---- ORBBEC Astra S deps (excluded from base — compilation fails on RPi4)
    libgflags-dev \
    nlohmann-json3-dev \
    libgoogle-glog-dev \
    # ---- libdc1394 runtime (needed by eYs3D prebuilt lib in depth_ydlidar_os30a)
    libdc1394-25 \
    # ---- OpenCV + video processing (camera drivers)
    python3-opencv \
    libopencv-dev \
    ffmpeg \
    # ---- Protobuf (required by camera drivers from sensor packages)
    protobuf-compiler \
    libprotobuf-dev \
    # ---- Python tooling + orion_chat system-level deps
    python3-venv \
    python3-soundfile \
    portaudio19-dev \
    && rm -rf /var/lib/apt/lists/*

# depth_ydlidar_os30a ships a prebuilt eYs3D lib compiled against libdc1394.so.22
# (Ubuntu 20/22.04 soname). Ubuntu 24.04 Noble ships libdc1394.so.25 — same API,
# renamed soname. The symlink makes the linker find the old name at build time.
RUN ln -s /usr/lib/x86_64-linux-gnu/libdc1394.so.25 \
          /usr/lib/x86_64-linux-gnu/libdc1394.so.22

# These packages are not in the rosdep database for Ubuntu Noble. Installing
# them here (at image build time) avoids failures in post_create.sh.
# --break-system-packages is required on Ubuntu 24.04 (PEP 668 enforced).
RUN pip3 install --no-cache-dir --break-system-packages \
    requests \
    openai-whisper \
    pygame \
    edge-tts \
    PyAudio \
    text2num \
    sounddevice \
    numpy \
    loguru \
    webrtcvad \
    pytest \
    jiwer

RUN mkdir -p /home/${USERNAME}/ws/src \
    && chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/ws

USER $USERNAME

RUN echo "source /home/${USERNAME}/ws/install/setup.bash 2>/dev/null || true" \
        >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

CMD ["/bin/bash"]
