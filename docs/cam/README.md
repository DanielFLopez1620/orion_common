# RPi Camera v1.3 (OV5647) en Ubuntu Server 24.04 — RPi 4
### Para uso con `camera_ros` en ROS 2 Jazzy | ORION Project — DFD Team

---

## Contexto

La **RPi Camera Module v1.3** usa el sensor **OV5647**. En Ubuntu Server 24.04, activarla y conectarla a ROS 2 Jazzy requiere varios pasos no obvios, principalmente porque:

- Ubuntu 24.04 incluye una versión *upstream* de `libcamera` que **no soporta hardware de Raspberry Pi**
- Los binarios `rpicam-*` no vienen de `libcamera` sino de un repo separado (`rpicam-apps`)
- El encoder `libav` incluido en `rpicam-apps` es incompatible con la versión de FFmpeg en Ubuntu 24.04
- Los permisos de `/dev/dma_heap` bloquean el acceso sin configuración adicional
- `ros-jazzy-camera-ros` instalado por apt trae su propia `libcamera` que **entra en conflicto** con el fork RPi compilado desde source, causando crashes de IPA proxy
- El encoding nativo del OV5647 (`NV21`) no es compatible con RViz ni rqt_image_view sin parámetros explícitos

---

## Prerrequisitos

- Raspberry Pi 4
- Ubuntu Server 24.04 (64-bit, aarch64)
- ROS 2 Jazzy instalado (`/opt/ros/jazzy`)
- RPi Camera Module v1.3 conectada al puerto CSI (ribbon cable con contactos metálicos orientados hacia los puertos HDMI)
- Acceso a internet desde la RPi

---

## Paso 1 — Conexión física

Conectar la cámara al puerto **CSI** de la RPi 4. El ribbon cable debe insertarse con los contactos metálicos apuntando hacia los puertos HDMI. Tirar suavemente del clip negro, insertar el cable y presionar el clip de vuelta.

> ⚠️ **Longitud máxima recomendada del ribbon:** hasta 50 cm para uso confiable. Cables más largos pueden funcionar pero son propensos a fallos de señal CSI (el sensor responde por I2C pero los frames hacen timeout). Para uso en robots móviles considerar adaptadores CSI-HDMI de Arducam que permiten hasta 10 m.

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
sudo dmesg | grep -i -E "camera|ov5647|unicam|csi"
```

Output esperado en dmesg:

```
# Sensor detectado correctamente — NO debe aparecer "i2c read error"
/soc/csi@7e801000: Fixed dependency cycle(s) with /soc/i2c0mux/i2c@1/ov5647@36
```

> ⚠️ Si aparece `ov5647_read: i2c read error, reg: 300a = -5` → el sensor no responde por I2C. Revisar conexión física del cable antes de continuar.

---

## Paso 4 — Eliminar libcamera del sistema (versión Ubuntu, sin soporte RPi)

Ubuntu 24.04 instala por defecto `libcamera 0.2.x`, que es la versión upstream **sin soporte para Raspberry Pi**. Debe eliminarse antes de compilar el fork oficial de RPi para evitar conflictos de linking.

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
# Debe mostrar entradas apuntando SOLO a /usr/local/lib/aarch64-linux-gnu
```

---

## Paso 7 — Compilar rpicam-apps

`rpicam-hello`, `rpicam-still`, `rpicam-vid` etc. son parte de un repo **separado** de `libcamera`.

> ⚠️ **Omisión necesaria:** El flag `-Denable_libav=disabled` es requerido porque la versión de `libavcodec` en Ubuntu 24.04 (v60.x) es incompatible con el encoder de `rpicam-apps`. Para el uso con `camera_ros` en ROS 2 esto no tiene impacto funcional.

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

Sin este paso, `rpicam-hello` y `camera_ros` fallan con `Could not open any dmaHeap device` para usuarios no-root.

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

## Paso 9 — Verificación de libcamera

```bash
rpicam-hello --list-cameras
```

Output esperado:

```
Available cameras
-----------------
0 : ov5647 [2592x1944] (/base/soc/i2c0mux/i2c@1/ov5647@36)
    Modes: 'SGBRG10_CSI2P' : 640x480 [58.92 fps] ...
```

Test de captura de imagen:

```bash
rpicam-still -o /tmp/test.jpg

# Transferir al PC para verificar visualmente:
# (desde el PC) scp ubuntu@<IP_RPI>:/tmp/test.jpg ~/Desktop/
```

---

## Paso 10 — Configurar camera_ros para ROS 2 Jazzy

### 10.1 — Eliminar ros-jazzy-libcamera (conflicto crítico)

`ros-jazzy-camera-ros` instalado por apt arrastra su propia `libcamera` en `/opt/ros/jazzy/lib/`. Esta versión **es ABI-incompatible** con el fork RPi compilado desde source, causando crashes del IPA proxy con este error:

```
FATAL Serializer control_serializer.cpp:626 A list of V4L2 controls requires a ControlInfoMap
ERROR IPAProxy raspberrypi_ipa_proxy.cpp:316 Failed to call start: -110
```

La causa es que el `raspberrypi_ipa_proxy` compilado desde source y la `libcamera.so` del apt hablan versiones distintas del protocolo de serialización — aunque ambas sean "0.7.0", son builds distintos con ABI incompatible.

Eliminar la libcamera de ROS:

```bash
sudo apt remove --purge ros-jazzy-libcamera -y
sudo ldconfig

# Verificar que ya no existe en el path de ROS
ls /opt/ros/jazzy/lib/libcamera.so* 2>/dev/null
ls /opt/ros/jazzy/lib/libcamera-base.so* 2>/dev/null
# Ambos deben retornar vacío

# Confirmar que ldconfig apunta solo a /usr/local/
ldconfig -p | grep "libcamera.so"
# Solo debe mostrar /usr/local/lib/aarch64-linux-gnu/
```

> ⚠️ Los archivos `libcamera_calibration_parsers.so` y `libcamera_info_manager.so` que permanecen en `/opt/ros/jazzy/lib/` son paquetes de ROS con nombre similar pero **completamente distintos** de libcamera — no tocarlos.

### 10.2 — Configurar variables de entorno

```bash
cat >> ~/.bashrc << 'EOF'
# libcamera RPi fork — evitar conflictos con versión de apt/ROS
export PKG_CONFIG_PATH=/usr/local/lib/aarch64-linux-gnu/pkgconfig:$PKG_CONFIG_PATH
export LIBCAMERA_IPA_MODULE_PATH=/usr/local/lib/aarch64-linux-gnu/libcamera
export LIBCAMERA_IPA_PROXY_PATH=/usr/local/libexec/libcamera
EOF
source ~/.bashrc
```

### 10.3 — Compilar camera_ros desde source

Dado que el paquete apt de `camera_ros` linkea contra la libcamera de ROS (incompatible), debe compilarse desde source para que use el fork RPi:

```bash
# Crear workspace
mkdir -p ~/ros2_ws/src && cd ~/ros2_ws

# Clonar camera_ros
git clone https://github.com/christianrauch/camera_ros.git src/camera_ros

# Remover camera_ros de apt si estaba instalado
sudo apt remove --purge ros-jazzy-camera-ros -y

# Compilar apuntando a /usr/local donde está el fork RPi
source /opt/ros/jazzy/setup.bash
colcon build --packages-select camera_ros \
  --cmake-args -DCMAKE_PREFIX_PATH="/usr/local"

# Sourcear el workspace
source ~/ros2_ws/install/setup.bash
```

Agregar el source al `.bashrc`:

```bash
echo 'source ~/ros2_ws/install/setup.bash' >> ~/.bashrc
source ~/.bashrc
```

---

## Paso 11 — Lanzar camera_ros

### Comando básico

```bash
ros2 run camera_ros camera_node
```

### Con parámetros explícitos (recomendado)

El OV5647 en su modo nativo usa `NV21` (YUV420 semi-planar), que **no es compatible** con RViz ni `rqt_image_view`. Usar `XRGB8888` para máxima compatibilidad:

```bash
ros2 run camera_ros camera_node --ros-args \
  -p format:=XRGB8888 \
  -p width:=640 \
  -p height:=480
```

### Resoluciones disponibles del OV5647

| Resolución | FPS | Uso recomendado |
|---|---|---|
| 640x480 | 58.92 fps | HRI tiempo real, detección de personas |
| 1296x972 | 43.25 fps | Balance calidad/rendimiento ✅ |
| 1920x1080 | 30.62 fps | Grabación, mapping |
| 2592x1944 | 15.63 fps | Máxima resolución, baja frecuencia |

### Formatos de pixel disponibles

| Formato | Compatibilidad | Notas |
|---|---|---|
| `XRGB8888` | RViz ✅, rqt ✅ | Recomendado para visualización |
| `YUYV` | rqt ✅ | Menor ancho de banda que XRGB |
| `NV21` | ❌ RViz/rqt | Formato nativo, requiere conversión explícita |

---

## Paso 12 — Verificar publicación en ROS 2

```bash
# En otra terminal
source /opt/ros/jazzy/setup.bash
source ~/ros2_ws/install/setup.bash

ros2 topic list
# Debe mostrar: /image_raw  /camera_info

ros2 topic hz /image_raw
# Verificar framerate

# Visualizar en rqt_image_view
ros2 run rqt_image_view rqt_image_view
```

> ℹ️ El warning `Camera calibration file [...] not found` es **normal** hasta que se realice la calibración con `camera_calibration`. No bloquea el funcionamiento básico del nodo.

---

## Troubleshooting

| Síntoma | Causa probable | Fix |
|---|---|---|
| `rpicam-hello: command not found` | Falta compilar `rpicam-apps` o PATH incorrecto | `export PATH=$PATH:/usr/local/bin` |
| `Could not open any dmaHeap device` | Permisos de `/dev/dma_heap` | Paso 8 completo + reconectar SSH |
| `No cameras available` después de permisos | `config.txt` incorrecto o cable mal conectado | Verificar Paso 2 y conexión física |
| Error `libavcodec API version is too old` | Incompatibilidad FFmpeg/rpicam-apps | Compilar con `-Denable_libav=disabled` |
| Dos versiones de libcamera en `ldconfig` | Ubuntu libcamera no eliminada | Paso 4 |
| `ninja` falla con Boost en `meson setup` | Falta `libboost-program-options-dev` | `sudo apt install libboost-program-options-dev` + `rm -rf build` |
| `Failed to call start: -110` en camera_ros | ABI mismatch libcamera ROS vs fork RPi | Paso 10.1: eliminar `ros-jazzy-libcamera` + recompilar |
| `FATAL Serializer` + crash IPA proxy | Misma causa que arriba | Mismo fix |
| `Unsupported image encoding [nv21]` en RViz | Formato nativo no compatible | Lanzar con `-p format:=XRGB8888` |
| Cámara detectada por I2C pero timeout en CSI | Cable dañado o puerto CSI dañado | Probar cable nuevo; si falla con 4+ cables distintos → módulo o puerto dañado |
| `i2c read error = -5` en dmesg | Sensor no responde físicamente | Revisar ribbon; si persiste → cámara dañada |

---

## Diagnóstico de hardware — ¿Módulo o puerto CSI dañado?

El OV5647 tiene dos buses independientes: **I2C** (configuración, señales lentas) y **CSI MIPI** (transferencia de frames, alta velocidad). Es posible que el sensor responda por I2C pero el transmisor CSI esté dañado por ESD u otro daño físico.

```bash
# Verificar respuesta I2C del sensor
sudo apt install i2c-tools
sudo i2cdetect -y 10
# "UU" en posición 0x36 → kernel lo tiene reservado, sensor responde
# "--" en posición 0x36 → sensor no responde = cámara dañada
```

Para separar si el problema es el módulo o el puerto CSI de la RPi: probar el mismo módulo en otra RPi. Si falla en ambas → módulo dañado. Si funciona en la otra → puerto CSI de la placa original dañado.

---

## Versiones verificadas

| Componente | Versión |
|---|---|
| Ubuntu Server | 24.04 LTS (aarch64) |
| Raspberry Pi | 4 |
| Sensor | OV5647 (RPi Camera v1.3) |
| libcamera (RPi fork) | 0.7.0 |
| rpicam-apps | 1.11.1 |
| ROS 2 | Jazzy |
| camera_ros | compilado desde source (github.com/christianrauch/camera_ros) |
| GCC | 13.3.0 |

---

*Documentado por DFD Team — ORION Project*