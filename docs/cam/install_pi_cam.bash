#!/bin/bash
# =============================================================================
# setup_rpicam_ubuntu2404.sh
# RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04 — RPi 4
# Para uso con v4l2_camera en ROS 2 Jazzy | ORION Project — DFD Team
# =============================================================================

set -e  # Salir si cualquier comando falla

# ─── Colores ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# ─── Helpers ──────────────────────────────────────────────────────────────────
info()    { echo -e "${BLUE}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn()    { echo -e "${YELLOW}[WARN]${NC} $1"; }
error()   { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }
step()    { echo -e "\n${CYAN}━━━ $1 ━━━${NC}"; }

# ─── Verificar que corremos en RPi 4 / Ubuntu 24.04 ──────────────────────────
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
  step "Paso 1/7: Configurando /boot/firmware/config.txt"

  CONFIG="/boot/firmware/config.txt"

  if [[ ! -f "$CONFIG" ]]; then
    error "No se encontró $CONFIG. ¿Estás en Ubuntu Server 24.04 para RPi?"
  fi

  # Backup
  sudo cp "$CONFIG" "${CONFIG}.bak.$(date +%Y%m%d_%H%M%S)"
  info "Backup creado: ${CONFIG}.bak.*"

  # Aplicar configuración
  if grep -q "dtoverlay=ov5647" "$CONFIG"; then
    info "dtoverlay=ov5647 ya existe en config.txt"
  else
    echo "" | sudo tee -a "$CONFIG" > /dev/null
    echo "# RPi Camera v1.3 (OV5647) — configurado por setup_rpicam_ubuntu2404.sh" | sudo tee -a "$CONFIG" > /dev/null
    echo "camera_auto_detect=0" | sudo tee -a "$CONFIG" > /dev/null
    echo "dtoverlay=ov5647" | sudo tee -a "$CONFIG" > /dev/null
    success "config.txt actualizado con camera_auto_detect=0 y dtoverlay=ov5647"
  fi

  # Desactivar camera_auto_detect si estaba en 1
  if grep -q "^camera_auto_detect=1" "$CONFIG"; then
    sudo sed -i 's/^camera_auto_detect=1/camera_auto_detect=0/' "$CONFIG"
    info "camera_auto_detect cambiado de 1 a 0"
  fi

  warn "Se requiere reiniciar. El script continuará al retomar después del reboot."
  warn "Después del reboot, vuelve a correr: sudo bash $0 --skip-boot"
}

# ─── Paso 2: Eliminar libcamera de apt ───────────────────────────────────────
remove_apt_libcamera() {
  step "Paso 2/7: Eliminando libcamera de apt (versión Ubuntu sin soporte RPi)"

  if dpkg -l | grep -q "libcamera0.2\|libcamera-dev"; then
    sudo apt remove --purge libcamera-dev libcamera0.2 -y
    sudo ldconfig
    success "libcamera de apt eliminada"
  else
    info "libcamera de apt no estaba instalada, continuando..."
  fi
}

# ─── Paso 3: Dependencias ─────────────────────────────────────────────────────
install_dependencies() {
  step "Paso 3/7: Instalando dependencias de compilación"

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
  step "Paso 4/7: Compilando libcamera (fork Raspberry Pi)"

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
  success "libcamera 0.7.0 (fork RPi) compilada e instalada"
}

_register_libcamera() {
  echo "/usr/local/lib/aarch64-linux-gnu" | sudo tee /etc/ld.so.conf.d/rpicam.conf > /dev/null
  sudo ldconfig
  info "Librerías registradas en ldconfig"
}

# ─── Paso 5: Compilar rpicam-apps ────────────────────────────────────────────
build_rpicam_apps() {
  step "Paso 5/7: Compilando rpicam-apps"
  info "Nota: libav encoder deshabilitado por incompatibilidad con FFmpeg Ubuntu 24.04"
  info "Esto no afecta el uso con v4l2_camera en ROS 2"

  cd ~

  if [[ -d "rpicam-apps" ]]; then
    warn "Directorio ~/rpicam-apps ya existe. ¿Recompilar desde cero?"
    read -rp "[s/N]: " confirm
    if [[ "$confirm" =~ ^[sS]$ ]]; then
      rm -rf rpicam-apps
    else
      info "Usando directorio existente..."
      cd rpicam-apps
      rm -rf build
    fi
  fi

  [[ ! -d ~/rpicam-apps ]] && git clone https://github.com/raspberrypi/rpicam-apps.git
  cd ~/rpicam-apps

  meson setup build --buildtype=release -Denable_libav=disabled
  ninja -C build
  sudo ninja -C build install
  sudo ldconfig

  success "rpicam-apps 1.11.1 compilado e instalado"
}

# ─── Paso 6: Permisos dma_heap ────────────────────────────────────────────────
configure_permissions() {
  step "Paso 6/7: Configurando permisos de dma_heap"

  # Agregar usuario al grupo video
  if groups "$USER" | grep -q "video"; then
    info "Usuario $USER ya pertenece al grupo video"
  else
    sudo usermod -aG video "$USER"
    success "Usuario $USER agregado al grupo video"
  fi

  # Regla udev
  UDEV_RULE='SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"'
  UDEV_FILE="/etc/udev/rules.d/99-dma-heap.rules"

  if [[ -f "$UDEV_FILE" ]]; then
    info "Regla udev ya existe en $UDEV_FILE"
  else
    echo "$UDEV_RULE" | sudo tee "$UDEV_FILE" > /dev/null
    success "Regla udev creada: $UDEV_FILE"
  fi

  sudo udevadm control --reload-rules
  sudo udevadm trigger
  success "Reglas udev recargadas"

  warn "IMPORTANTE: Cierra sesión y vuelve a conectar (SSH) para que el grupo 'video' aplique"
}

# ─── Paso 7: Verificación ─────────────────────────────────────────────────────
verify_installation() {
  step "Paso 7/7: Verificación"

  echo ""
  info "→ Dispositivos V4L2:"
  v4l2-ctl --list-devices 2>/dev/null || warn "v4l2-ctl no encontró dispositivos o falta reconectar sesión"

  echo ""
  info "→ Binario rpicam-hello:"
  which rpicam-hello && rpicam-hello --version || warn "rpicam-hello no encontrado en PATH"

  echo ""
  info "→ Cámaras detectadas por libcamera:"
  rpicam-hello --list-cameras 2>/dev/null || warn "No se pudo listar cámaras. Si ves 'dmaHeap', reconecta la sesión SSH primero."

  echo ""
  success "━━━ Setup completado ━━━"
  echo -e "${CYAN}Próximo paso:${NC} ros2 run v4l2_camera v4l2_camera_node --ros-args -p video_device:=/dev/video0"
}

# ─── Main ─────────────────────────────────────────────────────────────────────
main() {
  echo -e "${CYAN}"
  echo "╔══════════════════════════════════════════════════════════╗"
  echo "║   RPi Camera v1.3 (OV5647) — Ubuntu Server 24.04        ║"
  echo "║   Setup para v4l2_camera en ROS 2 Jazzy                  ║"
  echo "║   ORION Project — DFD Team                               ║"
  echo "╚══════════════════════════════════════════════════════════╝"
  echo -e "${NC}"

  # Modo --skip-boot para retomar después de reboot
  if [[ "$1" == "--skip-boot" ]]; then
    info "Modo post-reboot: saltando configuración de boot"
    remove_apt_libcamera
    install_dependencies
    build_libcamera
    build_rpicam_apps
    configure_permissions
    verify_installation

    echo ""
    warn "Recuerda cerrar sesión y reconectar SSH para que el grupo 'video' aplique."
    warn "Luego corre: rpicam-hello --list-cameras"
    return
  fi

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
}

main "$@"