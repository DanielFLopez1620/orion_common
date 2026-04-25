#!/bin/bash
# =============================================================================
# setup_rpicam_ubuntu2404.sh
# RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04 — RPi 4
# Para uso con camera_ros en ROS 2 Jazzy | ORION Project — DFD Team
#
# USO:
#   Primera corrida:  sudo bash setup_rpicam_ubuntu2404.sh
#   Post-reboot:      sudo bash setup_rpicam_ubuntu2404.sh --skip-boot
#   Solo ROS 2:       bash setup_rpicam_ubuntu2404.sh --ros-only
# =============================================================================

set -e

# ─── Colores ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ─── Helpers ──────────────────────────────────────────────────────────────────
info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
step()    { echo -e "\n${CYAN}━━━ $1 ━━━${NC}"; }

# ─── Verificar entorno ────────────────────────────────────────────────────────
check_environment() {
  step "Verificando entorno"

  if [[ "$(uname -m)" != "aarch64" ]]; then
    error "Este script requiere arquitectura aarch64 (RPi 4 con Ubuntu 64-bit)"
  fi

  if ! grep -q "24.04" /etc/os-release 2>/dev/null; then
    warn "Este script fue probado en Ubuntu 24.04. Continuar bajo tu propio riesgo."
    read -rp "¿Continuar de todas formas? [s/N]: " confirm
    [[ "$confirm" =~ ^[sS]$ ]] || exit 0
  fi

  success "Entorno verificado: aarch64 / Ubuntu 24.04"
}

# ─── Paso 1: config.txt ───────────────────────────────────────────────────────
configure_boot() {
  step "Paso 1/9: Configurando /boot/firmware/config.txt"

  CONFIG="/boot/firmware/config.txt"

  if [[ ! -f "$CONFIG" ]]; then
    error "No se encontró $CONFIG. ¿Estás en Ubuntu Server 24.04 para RPi?"
  fi

  sudo cp "$CONFIG" "${CONFIG}.bak.$(date +%Y%m%d_%H%M%S)"
  info "Backup creado: ${CONFIG}.bak.*"

  if grep -q "dtoverlay=ov5647" "$CONFIG"; then
    info "dtoverlay=ov5647 ya existe en config.txt"
  else
    echo "" | sudo tee -a "$CONFIG" > /dev/null
    echo "# RPi Camera v1.3 (OV5647) — configurado por setup_rpicam_ubuntu2404.sh" | sudo tee -a "$CONFIG" > /dev/null
    echo "camera_auto_detect=0" | sudo tee -a "$CONFIG" > /dev/null
    echo "dtoverlay=ov5647" | sudo tee -a "$CONFIG" > /dev/null
    success "config.txt actualizado"
  fi

  if grep -q "^camera_auto_detect=1" "$CONFIG"; then
    sudo sed -i 's/^camera_auto_detect=1/camera_auto_detect=0/' "$CONFIG"
    info "camera_auto_detect cambiado de 1 a 0"
  fi

  warn "Se requiere reiniciar. Después del reboot corre: sudo bash $0 --skip-boot"
}

# ─── Paso 2: Eliminar libcamera de apt ───────────────────────────────────────
remove_apt_libcamera() {
  step "Paso 2/9: Eliminando libcamera de apt (versión Ubuntu sin soporte RPi)"

  REMOVED=0
  for pkg in libcamera-dev libcamera0.2 libcamera0.5; do
    if dpkg -l | grep -q "^ii.*$pkg"; then
      sudo apt remove --purge "$pkg" -y
      REMOVED=1
    fi
  done

  if [[ $REMOVED -eq 1 ]]; then
    sudo ldconfig
    success "libcamera de apt eliminada"
  else
    info "libcamera de apt no estaba instalada, continuando..."
  fi
}

# ─── Paso 3: Dependencias ─────────────────────────────────────────────────────
install_dependencies() {
  step "Paso 3/9: Instalando dependencias de compilación"

  sudo apt update
  sudo apt install -y \
    git cmake meson ninja-build build-essential \
    python3-pip python3-yaml python3-ply \
    libboost-dev libboost-program-options-dev \
    libgnutls28-dev openssl \
    libjpeg-dev libtiff5-dev libpng-dev \
    libdrm-dev libexpat1-dev \
    libepoxy-dev libexif-dev \
    libavcodec-dev libavdevice-dev libavformat-dev libavutil-dev libswresample-dev \
    v4l-utils

  success "Dependencias instaladas"
}

# ─── Paso 4: Compilar libcamera (fork RPi) ───────────────────────────────────
build_libcamera() {
  step "Paso 4/9: Compilando libcamera (fork Raspberry Pi)"

  cd ~

  if [[ -d "libcamera" ]]; then
    warn "Directorio ~/libcamera ya existe. ¿Recompilar desde cero?"
    read -rp "[s/N]: " confirm
    if [[ "$confirm" =~ ^[sS]$ ]]; then
      rm -rf libcamera
    else
      info "Usando directorio existente, corriendo solo install..."
      cd libcamera
      sudo ninja -C build install
      _register_libcamera
      return
    fi
  fi

  git clone https://github.com/raspberrypi/libcamera.git
  cd libcamera
  meson setup build
  ninja -C build
  sudo ninja -C build install

  _register_libcamera
  success "libcamera (fork RPi) compilada e instalada"
}

_register_libcamera() {
  echo "/usr/local/lib/aarch64-linux-gnu" | sudo tee /etc/ld.so.conf.d/rpicam.conf > /dev/null
  sudo ldconfig
  info "Librerías registradas en ldconfig"
}

# ─── Paso 5: Compilar rpicam-apps ────────────────────────────────────────────
build_rpicam_apps() {
  step "Paso 5/9: Compilando rpicam-apps"
  info "Nota: libav encoder deshabilitado — incompatible con FFmpeg de Ubuntu 24.04 (v60.x)"

  cd ~

  if [[ -d "rpicam-apps" ]]; then
    warn "Directorio ~/rpicam-apps ya existe. ¿Recompilar desde cero?"
    read -rp "[s/N]: " confirm
    if [[ "$confirm" =~ ^[sS]$ ]]; then
      rm -rf rpicam-apps
    else
      info "Usando directorio existente..."
      cd rpicam-apps && rm -rf build
    fi
  fi

  [[ ! -d ~/rpicam-apps ]] && git clone https://github.com/raspberrypi/rpicam-apps.git
  cd ~/rpicam-apps

  meson setup build --buildtype=release -Denable_libav=disabled
  ninja -C build
  sudo ninja -C build install
  sudo ldconfig

  success "rpicam-apps compilado e instalado"
}

# ─── Paso 6: Permisos dma_heap ────────────────────────────────────────────────
configure_permissions() {
  step "Paso 6/9: Configurando permisos de dma_heap"

  if groups "$USER" | grep -q "video"; then
    info "Usuario $USER ya pertenece al grupo video"
  else
    sudo usermod -aG video "$USER"
    success "Usuario $USER agregado al grupo video"
  fi

  UDEV_FILE="/etc/udev/rules.d/99-dma-heap.rules"
  if [[ -f "$UDEV_FILE" ]]; then
    info "Regla udev ya existe"
  else
    echo 'SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"' | sudo tee "$UDEV_FILE" > /dev/null
    success "Regla udev creada"
  fi

  sudo udevadm control --reload-rules
  sudo udevadm trigger
  success "Reglas udev recargadas"

  warn "IMPORTANTE: Cierra sesión SSH y reconecta para que el grupo 'video' aplique"
}

# ─── Paso 7: Verificar libcamera ─────────────────────────────────────────────
verify_libcamera() {
  step "Paso 7/9: Verificación de libcamera"

  echo ""
  info "→ Dispositivos V4L2:"
  v4l2-ctl --list-devices 2>/dev/null || warn "v4l2-ctl no encontró dispositivos"

  echo ""
  info "→ Cámaras detectadas por libcamera:"
  rpicam-hello --list-cameras 2>/dev/null || warn "No se pudo listar cámaras. Reconecta la sesión SSH si ves 'dmaHeap'."

  echo ""
  success "Verificación de libcamera completada"
}

# ─── Paso 8: Configurar camera_ros para ROS 2 ────────────────────────────────
setup_ros2_camera() {
  step "Paso 8/9: Configurando camera_ros para ROS 2 Jazzy"

  # Verificar que ROS 2 está instalado
  if [[ ! -f "/opt/ros/jazzy/setup.bash" ]]; then
    error "ROS 2 Jazzy no encontrado en /opt/ros/jazzy. Instálalo primero."
  fi

  # 8.1 — Eliminar ros-jazzy-libcamera (causa ABI mismatch con fork RPi)
  info "Eliminando ros-jazzy-libcamera para evitar conflicto de ABI con fork RPi..."
  info "Este paquete causa: 'FATAL Serializer' y 'Failed to call start: -110'"

  for pkg in ros-jazzy-libcamera ros-jazzy-camera-ros; do
    if dpkg -l | grep -q "^ii.*$pkg"; then
      sudo apt remove --purge "$pkg" -y
      info "Eliminado: $pkg"
    fi
  done
  sudo ldconfig

  # Verificar que la libcamera de ROS ya no existe
  if ls /opt/ros/jazzy/lib/libcamera.so* 2>/dev/null | grep -q .; then
    warn "Aún existen libcamera.so en /opt/ros/jazzy/lib/ — puede haber conflictos"
  else
    success "Conflicto de libcamera eliminado"
  fi

  # 8.2 — Variables de entorno
  info "Configurando variables de entorno para libcamera fork RPi..."

  ENV_BLOCK="
# libcamera RPi fork — evitar conflictos con versión de apt/ROS
export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:\$PKG_CONFIG_PATH
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera"

  if ! grep -q "LIBCAMERA_IPA_MODULE_PATH" ~/.bashrc; then
    echo "$ENV_BLOCK" >> ~/.bashrc
    success "Variables de entorno agregadas a ~/.bashrc"
  else
    info "Variables de entorno ya existen en ~/.bashrc"
  fi

  export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
  export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
  export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera

  # 8.3 — Compilar camera_ros desde source
  info "Compilando camera_ros desde source (linkeará contra fork RPi de libcamera)..."

  mkdir -p ~/ros2_ws/src && cd ~/ros2_ws

  if [[ -d "src/camera_ros" ]]; then
    warn "src/camera_ros ya existe. ¿Recompilar desde cero?"
    read -rp "[s/N]: " confirm
    if [[ "$confirm" =~ ^[sS]$ ]]; then
      rm -rf src/camera_ros build/camera_ros install/camera_ros
      git clone https://github.com/christianrauch/camera_ros.git src/camera_ros
    fi
  else
    git clone https://github.com/christianrauch/camera_ros.git src/camera_ros
  fi

  source /opt/ros/jazzy/setup.bash
  colcon build --packages-select camera_ros \
    --cmake-args -DCMAKE_PREFIX_PATH="/usr/local"

  # Source del workspace
  if ! grep -q "ros2_ws/install/setup.bash" ~/.bashrc; then
    echo "source ~/ros2_ws/install/setup.bash" >> ~/.bashrc
    success "Workspace agregado al .bashrc"
  fi

  source ~/ros2_ws/install/setup.bash
  success "camera_ros compilado e instalado"
}

# ─── Paso 9: Verificación final ROS 2 ────────────────────────────────────────
verify_ros2() {
  step "Paso 9/9: Verificación final"

  source /opt/ros/jazzy/setup.bash
  source ~/ros2_ws/install/setup.bash 2>/dev/null || true

  echo ""
  info "→ Verificando que camera_ros existe:"
  ros2 pkg list | grep camera_ros || warn "camera_ros no encontrado en ros2 pkg list"

  echo ""
  success "━━━ Setup completo ━━━"

  echo ""
  echo -e "${CYAN}Comando de lanzamiento recomendado para OV5647:${NC}"
  echo ""
  echo "  ros2 run camera_ros camera_node --ros-args \\"
  echo "    -p format:=XRGB8888 \\"
  echo "    -p width:=640 \\"
  echo "    -p height:=480"
  echo ""
  echo -e "${CYAN}Resoluciones disponibles del OV5647:${NC}"
  echo "  640x480   @ 58.92 fps  → HRI tiempo real"
  echo "  1296x972  @ 43.25 fps  → balance calidad/rendimiento ✅"
  echo "  1920x1080 @ 30.62 fps  → grabación / mapping"
  echo "  2592x1944 @ 15.63 fps  → máxima resolución"
  echo ""
  echo -e "${CYAN}Verificar publicación:${NC}"
  echo "  ros2 topic list        # debe mostrar /image_raw y /camera_info"
  echo "  ros2 topic hz /image_raw"
  echo ""
  warn "Si ves 'Unsupported image encoding [nv21]' en RViz → usar -p format:=XRGB8888"
  warn "El warning 'calibration file not found' es normal hasta calibrar la cámara"
}

# ─── Main ─────────────────────────────────────────────────────────────────────
main() {
  echo -e "${CYAN}"
  echo "╔══════════════════════════════════════════════════════════╗"
  echo "║   RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04        ║"
  echo "║   Setup completo con camera_ros en ROS 2 Jazzy           ║"
  echo "║   ORION Project — DFD Team                               ║"
  echo "╚══════════════════════════════════════════════════════════╝"
  echo -e "${NC}"

  case "$1" in
    --skip-boot)
      info "Modo post-reboot: saltando configuración de boot"
      remove_apt_libcamera
      install_dependencies
      build_libcamera
      build_rpicam_apps
      configure_permissions
      verify_libcamera
      setup_ros2_camera
      verify_ros2
      echo ""
      warn "Cierra sesión SSH y reconecta para que el grupo 'video' aplique."
      warn "Luego lanza: ros2 run camera_ros camera_node --ros-args -p format:=XRGB8888 -p width:=640 -p height:=480"
      ;;

    --ros-only)
      info "Modo ROS only: configurando solo camera_ros (libcamera ya instalada)"
      setup_ros2_camera
      verify_ros2
      ;;

    *)
      check_environment
      configure_boot

      echo ""
      echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
      echo -e "${YELLOW}  Se requiere REBOOT para aplicar los cambios en config.txt   ${NC}"
      echo -e "${YELLOW}  Después del reboot, ejecuta:                                ${NC}"
      echo -e "${YELLOW}    sudo bash $(realpath $0) --skip-boot                      ${NC}"
      echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
      echo ""

      read -rp "¿Reiniciar ahora? [s/N]: " reboot_now
      if [[ "$reboot_now" =~ ^[sS]$ ]]; then
        sudo reboot
      fi
      ;;
  esac
}

main "$@"