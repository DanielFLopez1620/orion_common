#!/bin/bash

# Directorio de destino
DEST_DIR="comprimidas"

# Crear la carpeta si no existe
mkdir -p "$DEST_DIR"

# Iterar sobre imágenes que comienzan entre 1_ y 66_
for i in $(seq 1 9); do
    # Buscar imágenes que empiecen por i_
    for img in 0${i}_*.[jJ][pP][gG] 0${i}_*.[pP][nN][gG]; do
        # Verificar que el archivo existe (por si no hay matches)
        if [ -f "$img" ]; then
            # Obtener la extensión original en minúscula
            ext="${img##*.}"
            ext_lower=$(echo "$ext" | tr '[:upper:]' '[:lower:]')

            # Nombre de salida: solo el número y extensión
            output="$DEST_DIR/0${i}.${ext_lower}"

            # Comprimir imagen al 60% de calidad
            convert "$img" -quality 60 "$output"

            echo "✔️ Comprimida: $img → $output"
        fi
    done
done

echo "✅ Proceso completado. Archivos guardados en '$DEST_DIR'"
