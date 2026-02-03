# Project Report: Distributed Process Management System

## Overview
This project implements a distributed system capable of managing operating system processes remotely. It fulfills the requirements of process management, network communication, and middleware-based distribution (via service discovery).

## Implementation Details

### Task 1: Structure & Process Management
*   **Module**: `src/server/main.py` (ProcessManager class)
*   **Functionality**:
    *   **Listing**: Uses `ps` (Unix) or `tasklist` (Windows) to parse active processes.
    *   **Starting**: Uses `subprocess.Popen` to launch non-blocking commands.
    *   **Stopping**: Uses `os.kill` with signal 9 (SIGKILL) to terminate processes.
    *   **Monitoring**: Checks process existence using signal 0.

### Task 2: Network Communication (TCP/IP)
*   **Protocol**: TCP Sockets.
*   **Server**: Multithreaded server (`threading.Thread`) handling one client per thread.
*   **Client**: Connects to the server IP/Port and sends text-based commands.
*   **Data Exchange**: Commands are sent as strings; responses are text or JSON (for lists).

### Task 3: Distributed Systems (Middleware/Discovery)
*   **Mechanism**: UDP Broadcast.
*   **Workflow**:
    1.  Server listens on UDP port 5001.
    2.  Client sends `DISCOVER_SERVERS` to `<broadcast>`.
    3.  Server responds with its TCP port.
    4.  Client aggregates responses and allows the user to choose a target server.
*   This acts as a simple "Directory Service" or Middleware layer allowing dynamic connection without hardcoded IPs.

## Source Code Organization

```
.
├── README.md           # Instructions
├── REPORT.md           # This report
├── src
│   ├── client
│   │   └── main.py     # Client application
│   └── server
│       └── main.py     # Server application with ProcessManager
└── docs                # Documentation folder
```

## Future Improvements
*   **Security**: Add authentication (SSL/TLS or tokens) to prevent unauthorized process manipulation.
*   **Advanced Monitoring**: Integrate `psutil` library for real-time CPU/Memory percentages.
*   **Concurrency**: Use `asyncio` for better scalability on the server side.
