FROM orion_base:latest

ARG USERNAME=orion_user
ARG USER_UID=1001

USER root

RUN apt-get update && apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    ros-${ROS_DISTRO}-v4l2-camera \
    libgflags-dev \
    nlohmann-json3-dev \
    libgoogle-glog-dev \
    libdc1394-25 \
    python3-opencv \
    libopencv-dev \
    ffmpeg \
    protobuf-compiler \
    libprotobuf-dev \
    python3-venv \
    python3-soundfile \
    portaudio19-dev \
    && rm -rf /var/lib/apt/lists/*

RUN MULTIARCH="$(dpkg-architecture -qDEB_HOST_MULTIARCH)" \
    && ln -s "/usr/lib/${MULTIARCH}/libdc1394.so.25" \
             "/usr/lib/${MULTIARCH}/libdc1394.so.22"

RUN pip3 install --no-cache-dir --break-system-packages \
    --default-timeout=1000 --retries 5 \
    requests \
    numpy \
    pytest

RUN mkdir -p /home/${USERNAME}/ws/src \
    && chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/ws

USER $USERNAME

RUN echo "source /home/${USERNAME}/ws/install/setup.bash 2>/dev/null || true" \
        >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

CMD ["/bin/bash"]
