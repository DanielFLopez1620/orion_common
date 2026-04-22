# RPi Camera v1.3 (OV5647) en Ubuntu Server 24.04 — RPi 4
### Para uso con `v4l2_camera` en ROS 2 Jazzy | ORION Project — DFD Team

---

## Contexto

La **RPi Camera Module v1.3** usa el sensor **OV5647**. En Ubuntu Server 24.04, activarla para usarla como dispositivo V4L2 (requerido por el nodo `v4l2_camera` de ROS 2) requiere varios pasos no obvios, principalmente porque:

- Ubuntu 24.04 incluye una versión *upstream* de `libcamera` que **no soporta hardware de Raspberry Pi**
- Los binarios `rpicam-*` no vienen de `libcamera` sino de un repo separado (`rpicam-apps`)
- El encoder `libav` incluido en `rpicam-apps` es incompatible con la versión de FFmpeg en Ubuntu 24.04
- Los permisos de `/dev/dma_heap` bloquean el acceso sin configuración adicional

---

## Prerrequisitos

- Raspberry Pi 4
- Ubuntu Server 24.04 (64-bit, aarch64)
- RPi Camera Module v1.3 conectada al puerto CSI (ribbon cable con contactos metálicos orientados hacia los puertos HDMI)
- Acceso a internet desde la RPi

---

## Paso 1 — Conexión física

Conectar la cámara al puerto **CSI** de la RPi 4. El ribbon cable debe insertarse con los contactos metálicos apuntando hacia los puertos HDMI. Tirar suavemente del clip negro, insertar el cable y presionar el clip de vuelta.

---

## Paso 2 — Configurar `/boot/firmware/config.txt`

```bash
sudo nano /boot/firmware/config.txt
```

Agregar o modificar:

```ini
# Deshabilitar autodetección (causa conflictos con OV5647 en Ubuntu)
camera_auto_detect=0

# Cargar el overlay del sensor OV5647
dtoverlay=ov5647
```

> ⚠️ El archivo correcto en Ubuntu 24.04 es `/boot/firmware/config.txt`, **no** `/boot/config.txt`.
> Con `camera_auto_detect=1` sin el dtoverlay explícito, el dispositivo `/dev/video0` no se crea correctamente.

Reiniciar:

```bash
sudo reboot
```

---

## Paso 3 — Verificar detección a nivel de kernel

```bash
ls -l /dev/video*
v4l2-ctl --list-devices
dmesg | grep -i -E "camera|ov5647|unicam|csi"
```

Output esperado:

```
unicam (platform:fe801000.csi):
    /dev/video0
    /dev/media0
```

---

## Paso 4 — Eliminar libcamera del sistema (versión Ubuntu, sin soporte RPi)

Ubuntu 24.04 instala por defecto `libcamera 0.2.0`, que es la versión upstream **sin soporte para Raspberry Pi**. Debe eliminarse antes de compilar el fork oficial de RPi para evitar conflictos de linking.

```bash
sudo apt remove --purge libcamera-dev libcamera0.2 -y
sudo ldconfig
```

Verificar que no quedan restos:

```bash
ldconfig -p | grep libcamera
# No debe mostrar nada, o solo paths de /usr/local/
```

---

## Paso 5 — Instalar dependencias de compilación

```bash
sudo apt update && sudo apt full-upgrade -y
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
```

---

## Paso 6 — Compilar libcamera (fork de Raspberry Pi)

```bash
cd ~
git clone https://github.com/raspberrypi/libcamera.git
cd libcamera
meson setup build
ninja -C build
sudo ninja -C build install
```

Registrar las librerías instaladas:

```bash
echo "/usr/local/lib/aarch64-linux-gnu" | sudo tee /etc/ld.so.conf.d/rpicam.conf
sudo ldconfig
```

Verificar:

```bash
ldconfig -p | grep libcamera
# Debe mostrar entradas apuntando a /usr/local/lib/aarch64-linux-gnu
```

---

## Paso 7 — Compilar rpicam-apps

`rpicam-hello`, `rpicam-still`, `rpicam-vid` etc. son parte de un repo **separado** de `libcamera`.

> ⚠️ **Omisión necesaria:** El flag `-Denable_libav=disabled` es requerido porque la versión de `libavcodec` en Ubuntu 24.04 (v60.x) es incompatible con el encoder de `rpicam-apps`. Para el uso con `v4l2_camera` en ROS 2 esto no tiene impacto funcional.

```bash
cd ~
git clone https://github.com/raspberrypi/rpicam-apps.git
cd rpicam-apps
meson setup build --buildtype=release -Denable_libav=disabled
ninja -C build
sudo ninja -C build install
sudo ldconfig
```

Verificar binarios:

```bash
which rpicam-hello
rpicam-hello --version
```

---

## Paso 8 — Configurar permisos de dma_heap

Sin este paso, `rpicam-hello` falla con `Could not open any dmaHeap device` para usuarios no-root.

```bash
# Agregar usuario al grupo video
sudo usermod -aG video $USER

# Crear regla udev
echo 'SUBSYSTEM=="dma_heap", GROUP="video", MODE="0660"' | \
  sudo tee /etc/udev/rules.d/99-dma-heap.rules

# Aplicar reglas
sudo udevadm control --reload-rules
sudo udevadm trigger
```

**Cerrar sesión y volver a conectar por SSH** para que el grupo aplique:

```bash
exit
# reconectar...
groups  # debe incluir "video"
```

---

## Paso 9 — Verificación final

```bash
# Listar cámaras detectadas
rpicam-hello --list-cameras
```

Output esperado:

```
Available cameras
-----------------
0 : ov5647 [2592x1944] (/base/soc/i2c0mux/i2c@1/ov5647@36)
    Modes: 'SGBRG10_CSI2P' : 640x480 [58.92 fps] ...
```

```bash
# Verificar dispositivo V4L2
v4l2-ctl --list-devices
v4l2-ctl --list-formats -d /dev/video0
```

---

## Siguiente paso: v4l2_camera en ROS 2 Jazzy

Con `/dev/video0` activo y verificado, el nodo `v4l2_camera` puede conectarse. Consideraciones:

- El formato nativo del OV5647 es Bayer RAW (`SGBRG10`), el nodo requiere configuración del formato de pixel
- Instalar: `sudo apt install ros-jazzy-v4l2-camera`
- Lanzar: `ros2 run v4l2_camera v4l2_camera_node --ros-args -p video_device:=/dev/video0`

---

## Troubleshooting

| Síntoma | Causa probable | Fix |
|---|---|---|
| `rpicam-hello: command not found` | Falta compilar `rpicam-apps` o PATH incorrecto | `export PATH=$PATH:/usr/local/bin` |
| `Could not open any dmaHeap device` | Permisos de `/dev/dma_heap` | Paso 8 completo |
| `No cameras available` después de permisos | `config.txt` incorrecto o cable mal conectado | Verificar Paso 2 y conexión física |
| Error `libavcodec API version is too old` | Incompatibilidad FFmpeg/rpicam-apps | Compilar con `-Denable_libav=disabled` |
| Dos versiones de libcamera en `ldconfig` | Ubuntu libcamera no eliminada | Paso 4 |
| `ninja` falla en `meson setup` con Boost | Falta `libboost-program-options-dev` | `sudo apt install libboost-program-options-dev` + `rm -rf build` |

---

## Versiones verificadas

| Componente | Versión |
|---|---|
| Ubuntu Server | 24.04 LTS (aarch64) |
| Raspberry Pi | 4 |
| Sensor | OV5647 (RPi Camera v1.3) |
| libcamera (RPi fork) | 0.7.0 |
| rpicam-apps | 1.11.1 |
| GCC | 13.3.0 |

---

*Documentado por DFD Team — ORION Project*