# Process Management System

A distributed client-server application for managing processes across a network.

## Features

1.  **Process Management**: List, start, stop, and monitor processes.
2.  **Networked Architecture**: TCP/IP socket communication.
3.  **Service Discovery**: UDP Broadcast to find servers automatically.

## Requirements

*   Python 3.x

## Usage

### 1. Start the Server
Run the server on the machine(s) you want to manage.

```bash
python3 src/server/main.py
```

### 2. Start the Client
Run the client on any machine in the same network.

```bash
python3 src/client/main.py
```

The client will automatically scan for available servers. Select one to connect to.

### Commands

*   `LIST`: Show active processes.
*   `START <command>`: Execute a shell command (e.g., `START sleep 10`).
*   `STOP <pid>`: Kill a process by PID.
*   `MONITOR <pid>`: Check if a process is running.
*   `EXIT`: Close connection.
