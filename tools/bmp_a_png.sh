#!/bin/sh
# Convierte las figuras capturas/fig*.bmp a PNG (para pegarlas en el informe).
# Usa sips (viene con macOS) o ImageMagick si esta instalado.
cd "$(dirname "$0")/../capturas" || exit 1
for f in fig*.bmp; do
    [ -e "$f" ] || continue
    if command -v sips >/dev/null 2>&1; then
        sips -s format png "$f" --out "${f%.bmp}.png" >/dev/null
    elif command -v magick >/dev/null 2>&1; then
        magick "$f" "${f%.bmp}.png"
    fi
done
echo "Figuras en PNG: $(ls fig*.png 2>/dev/null | wc -l | tr -d ' ')"
