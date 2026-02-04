#!/bin/bash

# Compilar todo
make

# Directorio de instalación (debe estar en el PATH)
# En AWS usualmente /usr/local/bin es ideal para esto
INSTALL_DIR="/usr/local/bin"

echo "Creando symlinks en $INSTALL_DIR..."
echo "Nota: Se requieren permisos de sudo para escribir en $INSTALL_DIR"

sudo ln -sf "$(pwd)/bin/hola" "$INSTALL_DIR/hola"
sudo ln -sf "$(pwd)/bin/juego" "$INSTALL_DIR/juego"
sudo ln -sf "$(pwd)/bin/v21" "$INSTALL_DIR/v21"

echo "¡Listo! Ahora puedes usar START hola, START juego o START v21"
