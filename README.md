# Remote Process Manager (C/POSIX)

Este proyecto es un sistema de administración de procesos distribuido desarrollado en C. Permite a un cliente remoto conectarse a un servidor (por ejemplo, en AWS EC2) para listar, iniciar y detener procesos del sistema operativo.

## Características Principales

*   **Alto Rendimiento**: Desarrollado íntegramente en C para un consumo mínimo de recursos.
*   **Capacidad Extendida**: Buffers de 64KB para manejar listas de procesos completas.
*   **Multihilo**: El servidor puede atender a múltiples clientes simultáneamente usando hilos POSIX.
*   **Gestión Real de Procesos**: Usa `fork/exec` para lanzar comandos reales del sistema.
*   **Despliegue en AWS**: Optimizado para conexiones directas TCP a través de firewalls (Security Groups).

## Requisitos

*   Compilador `gcc`
*   Herramienta `make`
*   Sistema operativo basado en POSIX (Linux/Ubuntu para AWS, macOS para local).

## Compilación e Instalación

### En el Servidor (AWS/Linux):
1. **Compilar e instalar comandos globales**:
   ```bash
   make -f Makefile.server install
   ```
   *Esto genera el ejecutable `server_bin`, compila los juegos/scripts en `bin/` y crea los accesos directos (symlinks) en `/usr/local/bin`.*

### En el Cliente (Local):
1. **Compilar**:
   ```bash
   make -f Makefile.client
   ```

## Configuración del Servicio (Systemd)

Para que el servidor corra como un demonio en AWS, crea el archivo `/etc/systemd/system/proc-manager.service`:

```ini
[Unit]
Description=Remote Process Manager Server
After=network.target

[Service]
WorkingDirectory=/root/avanceProyecto
ExecStart=/root/avanceProyecto/server_bin
Restart=always
User=root

[Install]
WantedBy=multi-user.target
```

**Comandos útiles:**
* `sudo systemctl daemon-reload`
* `sudo systemctl enable proc-manager`
* `sudo systemctl start proc-manager`
* `sudo systemctl status proc-manager`

## Uso del Cliente

Ejecuta el cliente y proporciona la IP de tu servidor:
```bash
./client_bin
```

### Comandos Disponibles
*   `LIST`: Muestra **todos** los procesos activos en el servidor (hasta 64KB de datos).
*   `START <comando>`: Inicia un proceso (ej. `START v21`, `START sleep 100`).
*   `STOP <pid>`: Detiene un proceso usando su ID.
*   `EXIT`: Finaliza la sesión.

## Notas de Seguridad (AWS)
Asegúrate de abrir el puerto **TCP 5002** en el **Security Group** de tu instancia.
