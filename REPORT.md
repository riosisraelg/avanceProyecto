# Project Report: Distributed Process Management System (C-POSIX Version)

## Overview
Sistema distribuido de gestión de procesos remotos desarrollado íntegramente en **C (POSIX)**. Optimizado para alto rendimiento en **AWS EC2**, permitiendo la administración segura y concurrente de procesos mediante sockets TCP.

## Implementation Details

### Task 1: Gestión de Procesos
*   **Listing**: Captura de procesos activos vía `ps` y `popen()`.
*   **Starting**: Ejecución asíncrona mediante `fork()` y `execvp()`.
*   **Stopping**: Terminación inmediata de procesos con `kill()` y `SIGKILL`.

### Task 2: Comunicación y Concurrencia
*   **TCP/IP**: Comunicación fiable orientada a conexión.
*   **Multithreading**: Manejo de múltiples clientes mediante `pthreads`. Cada conexión se procesa en un hilo independiente y "detached" para evitar fugas de memoria.

### Task 3: Extensibilidad (Comandos Personalizados)
*   **Binarios**: Programas en C independientes (`hola`, `juego`, `v21`).
*   **Integración**: Uso de `Makefile.server` para instalar enlaces simbólicos en `/usr/local/bin`, permitiendo la ejecución global de los comandos desde el servidor.

## Estructura Final del Proyecto

```
.
├── README.md           # Guía de usuario y despliegue
├── REPORT.md           # Este reporte
├── Makefile.server     # Compilación e instalación del servidor
├── Makefile.client     # Compilación del cliente
├── bin                 # Binarios compilados (generado por Makefile)
└── src
    ├── client
    │   └── main.c      # Código fuente Cliente
    ├── server
    │   └── main.c      # Código fuente Servidor
    └── commands
        ├── hola.c      # Lógica de impresión lenta
        ├── juego.c     # Lógica de juego de azar
        └── v21.c       # Lógica de simulación 21
```

## Casos de Uso
1.  **Monitoreo Remoto**: Consulta de estado de la instancia AWS.
2.  **Ejecución de Tareas**: Lanzamiento de scripts y programas personalizados.
3.  **Control de Recursos**: Finalización de procesos que comprometan la estabilidad del servidor.
