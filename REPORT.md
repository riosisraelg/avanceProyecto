# Project Report: Distributed Process Management System (C-POSIX Version)

## Overview
Sistema distribuido de gestión de procesos remotos desarrollado íntegramente en **C (POSIX)**. Diseñado para ofrecer una interfaz ligera de administración sobre instancias de **AWS EC2**, garantizando concurrencia mediante hilos y eficiencia en la transferencia de datos.

## Detalles Técnicos Avanzados

### 1. Gestión de Procesos y Escalabilidad
*   **Listing Completo**: Se eliminó la restricción de 20 líneas. Ahora el servidor captura la salida completa de `ps -e -o pid,comm` y la transmite íntegramente.
*   **Transmisión de Datos**: Se implementaron buffers de **65,536 bytes (64 KB)** tanto en cliente como en servidor para evitar el truncamiento de datos en sistemas con una carga alta de procesos.

### 2. Concurrencia y Sockets
*   **Modelo Multihilo**: Uso de `pthreads` con el atributo `detached`. Esto asegura que cada hilo de cliente sea recolectado por el sistema operativo al terminar, evitando fugas de memoria (*memory leaks*).
*   **Protocolo TCP**: Se utiliza un socket de flujo persistente que permite múltiples comandos por sesión antes del cierre (`EXIT`).

### 3. Automatización y Despliegue
*   **Integración en Makefile**: La lógica de instalación de comandos personalizados fue migrada de scripts Bash directamente al `Makefile.server` bajo el objetivo `install`.
*   **Demonización**: El sistema está preparado para operar como un servicio de sistema (`systemd`), garantizando alta disponibilidad y reinicio automático ante fallos.

## Estructura Final

```
.
├── README.md           # Guía de usuario y configuración de systemd
├── REPORT.md           # Este reporte técnico
├── Makefile.server     # Compilación e instalación global de symlinks
├── Makefile.client     # Compilación del binario de administración
├── bin                 # Binarios de comandos (hola, juego, v21)
└── src
    ├── client
    │   └── main.c      # Cliente TCP interactivo
    ├── server
    │   └── main.c      # Servidor multihilo y gestor de fork/exec
    └── commands
        ├── hola.c      # Comando de prueba de latencia
        ├── juego.c     # Juego interactivo de adivinanza
        └── v21.c       # Simulación de Blackjack 21
```

## Conclusión
La migración a C ha resultado en una herramienta significativamente más potente y ligera que la versión original en Python, permitiendo un control total sobre las primitivas del sistema operativo y una integración natural con los servicios de administración de Linux en la nube.