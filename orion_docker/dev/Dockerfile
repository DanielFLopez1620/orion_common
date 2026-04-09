FROM orion_base:latest

# Match the user defined in base — override with --build-arg if needed
ARG USERNAME=orion_user
ARG USER_UID=1001

# Switch to root for system-level installations
USER root

# ── 1. Dev-only packages ──────────────────────────────────────────────────────
# Does NOT repeat what orion_base already installs.
RUN apt-get update && apt-get install -y --no-install-recommends \
    # ---- Simulation + visualization (PC only — too heavy for RPi)
    ros-${ROS_DISTRO}-ros-gz \
    ros-${ROS_DISTRO}-rviz2 \
    ros-${ROS_DISTRO}-joint-state-publisher-gui \
    # ---- ORBBEC Astra S deps (excluded from base — compilation fails on RPi4)
    libgflags-dev \
    nlohmann-json3-dev \
    # ---- OpenCV + video processing (camera drivers)
    python3-opencv \
    libopencv-dev \
    ffmpeg \
    # ---- Protobuf (required by camera drivers from sensor packages)
    protobuf-compiler \
    libprotobuf-dev \
    # ---- Python tooling
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

# ── 2. Workspace setup ────────────────────────────────────────────────────────
# orion_common is bind-mounted by devcontainer at runtime — no clone needed.
# Other repos are handled by post_create.sh via vcs import.
RUN mkdir -p /home/${USERNAME}/ws/src \
    && chown -R ${USERNAME}:${USERNAME} /home/${USERNAME}/ws

USER $USERNAME

RUN echo "source /home/${USERNAME}/ws/install/setup.bash 2>/dev/null || true" \
        >> /home/${USERNAME}/.bashrc

WORKDIR /home/${USERNAME}/ws

CMD ["/bin/bash"]
