# Remote Process Manager (C/POSIX)

Este proyecto es un sistema de administración de procesos distribuido desarrollado en C. Permite a un cliente remoto conectarse a un servidor (por ejemplo, en AWS EC2) para listar, iniciar y detener procesos del sistema operativo.

## Características Principales

*   **Alto Rendimiento**: Desarrollado íntegramente en C para un consumo mínimo de recursos.
*   **Multihilo**: El servidor puede atender a múltiples clientes simultáneamente usando hilos POSIX.
*   **Gestión Real de Procesos**: Usa `fork/exec` para lanzar comandos reales del sistema.
*   **Despliegue en AWS**: Optimizado para conexiones directas TCP a través de firewalls (Security Groups).

## Requisitos

*   Compilador `gcc`
*   Herramienta `make`
*   Sistema operativo basado en POSIX (Linux/Ubuntu para AWS, macOS para local).

## Compilación

El proyecto cuenta con Makefiles independientes para facilitar el despliegue:

### En el Servidor (AWS/Linux):
```bash
make -f Makefile.server
```
Esto generará el ejecutable `server_bin` y los comandos adicionales en la carpeta `bin/`.

### En el Cliente (Local):
```bash
make -f Makefile.client
```
Esto generará el ejecutable `client_bin`.

## Configuración y Ejecución

### 1. Preparar el Servidor
Para que el servidor reconozca los comandos personalizados (`hola`, `juego`, `v21`), ejecuta el script de configuración para crear los accesos directos:

```bash
chmod +x scripts/setup_path.sh
./scripts/setup_path.sh
```

Inicia el servidor:
```bash
./server_bin
```

### 2. Conectar el Cliente
Ejecuta el cliente y proporciona la IP de tu servidor:
```bash
./client_bin
```

## Comandos Disponibles

*   `LIST`: Muestra los primeros 20 procesos activos en el servidor.
*   `START <comando>`: Inicia un proceso (ej. `START v21`, `START sleep 100`).
*   `STOP <pid>`: Detiene un proceso usando su ID.
*   `EXIT`: Finaliza la sesión.

## Notas de Seguridad (AWS)
Asegúrate de abrir los siguientes puertos en el **Security Group** de tu instancia:
*   **TCP 5002**: Entrada para el tráfico del administrador.