FROM docker.io/osrf/ros:jazzy-desktop-full
ARG USERNAME=orion
ARG USER_UID=1010
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME
RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y python3-pip python3-setuptools python3-vcstool \ 
    apt-utils bash-completion \
    curl wget software-properties-common

    RUN apt-get update && apt-get install -y \
    python3-opencv \
    python3-venv \
    ffmpeg \
    protobuf-compiler \
    libprotobuf-dev \
    cmake \
    libopencv-dev \
    git \
    && apt-get clean

# Install MediaPipe
RUN pip3 install mediapipe --break-system-packages

ENV SHELL=/bin/bash

RUN colcon mixin remove default
RUN colcon mixin add default https://raw.githubusercontent.com/colcon/colcon-mixin-repository/master/index.yaml
RUN colcon mixin update default

RUN apt-get update && apt-get install -y python3-colcon-common-extensions python3-colcon-mixin

RUN apt-get update && apt-get install -y ros-$ROS_DISTRO-rmw-cyclonedds-cpp

RUN apt-get update && apt-get install -y ros-$ROS_DISTRO-ros-gz


COPY repos.yaml /home/$USERNAME/ws/repos.yaml

USER $USERNAME

RUN echo "source /opt/ros/jazzy/setup.bash" >> /home/orion/.bashrc
RUN echo "source /home/orion/ws/install/setup.bash || echo 'Workspace not Ready. Run colcon build at /home/orion/ws'" >> /home/orion/.bashrc
RUN echo "export RMW_IMPLEMENTATION=rmw_cyclonedds_cpp" >> /home/orion/.bashrc

RUN sudo apt update && sudo apt upgrade -y

WORKDIR /home/$USERNAME/ws

CMD ["/bin/bash"]