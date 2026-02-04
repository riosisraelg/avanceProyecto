# Project Report: Distributed Process Management System (C-POSIX Version)

## Overview
Este proyecto implementa un sistema distribuido de gestión de procesos remotos migrado de Python a **C (POSIX)** para optimizar el rendimiento y la eficiencia en entornos de servidor como **AWS EC2**. El sistema permite a clientes externos administrar, monitorear y ejecutar tareas en un servidor remoto mediante una conexión TCP persistente.

## Implementation Details

### Task 1: Gestión de Procesos (Server-side)
*   **Módulo**: `src/server/main.c`
*   **Funcionalidad**:
    *   **Listing**: Ejecuta comandos del sistema (`ps`) mediante `popen()` para capturar la salida de los procesos activos.
    *   **Starting**: Utiliza la primitiva `fork()` para crear un proceso hijo y `execvp()` para reemplazar la imagen del proceso con el comando solicitado. La salida se redirige a `/dev/null` para mantener la independencia del servidor.
    *   **Stopping**: Implementa la llamada al sistema `kill()` con la señal `SIGKILL` (9) para asegurar la detención inmediata del PID especificado.

### Task 2: Comunicación de Red (TCP/IP Multihilo)
*   **Protocolo**: TCP Sockets.
*   **Arquitectura del Servidor**: Basada en hilos nativos (`pthreads`). El servidor principal acepta conexiones y delega cada cliente a un hilo independiente (`pthread_create` + `pthread_detach`), permitiendo múltiples administradores simultáneos sin bloqueo.
*   **Optimización AWS**: Se eliminó el descubrimiento UDP (broadcast) debido a que los clientes externos a la VPC no pueden usar tráfico de difusión en internet, optando por una conexión directa vía IP/DNS.

### Task 3: Comandos Personalizados y Extensiones
*   Se desarrollaron micro-programas en C (`src/commands/`) para demostrar la capacidad de ejecución:
    *   `hola`: Demostración de latencia controlada (impresión letra a letra).
    *   `juego`: Lógica de interactividad básica.
    *   `v21`: Simulación de juego de cartas 21.
*   **Integración**: Se utilizan *symlinks* en el PATH del sistema para permitir la ejecución de estos comandos mediante nombres cortos (ej. `START v21`).

## Estructura del Proyecto

```
.
├── Makefile.server     # Compilación exclusiva del servidor y comandos
├── Makefile.client     # Compilación exclusiva del cliente
├── scripts
│   └── setup_path.sh   # Configuración de symlinks y entorno
├── bin                 # Binarios de comandos personalizados
├── src
│   ├── client
│   │   └── main.c      # Cliente en C (Interactivo)
│   ├── server
│   │   └── main.c      # Servidor multihilo en C
│   └── commands
│       ├── hola.c      # Comando: Hola Mundo Lento
│       ├── juego.c     # Comando: Adivinar número
│       └── v21.c       # Comando: Blackjack 21
└── REPORT.md           # Este reporte
```

## Casos de Uso
1.  **Monitoreo de Salud**: Uso de `LIST` para verificar que servicios críticos en la instancia AWS sigan activos.
2.  **Mantenimiento Remoto**: Ejecución de scripts de limpieza o respaldos mediante `START`.
3.  **Recuperación de Desastres**: Detención de procesos zombies o fuera de control que consumen CPU excesiva mediante `STOP`.