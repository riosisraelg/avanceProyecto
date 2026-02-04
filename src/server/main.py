import socket
import threading
import subprocess
import os
import platform
import json
import time

# Configuration
HOST = '0.0.0.0'
TCP_PORT = 5002
UDP_PORT = 5001
BUFFER_SIZE = 4096

class ProcessManager:
    def list_processes(self):
        """Lists processes using OS specific commands."""
        system = platform.system()
        procs = []
        try:
            if system == "Windows":
                # Basic tasklist implementation for Windows
                cmd = "tasklist /FO CSV"
                output = subprocess.check_output(cmd, shell=True).decode()
                lines = output.strip().split("\r\n")[1:] # Skip header
                for line in lines:
                    parts = line.split('","')
                    if len(parts) >= 2:
                        name = parts[0].strip('"')
                        pid = parts[1].strip('"')
                        procs.append({"pid": pid, "name": name})
            else:
                # Unix/Linux/MacOS
                cmd = ["ps", "-e", "-o", "pid,comm"]
                output = subprocess.check_output(cmd).decode()
                lines = output.strip().split("\n")[1:]
                for line in lines:
                    parts = line.strip().split(maxsplit=1)
                    if len(parts) == 2:
                        procs.append({"pid": parts[0], "name": parts[1]})
            return procs[:20] # Return top 20 to avoid flooding network for this demo
        except Exception as e:
            return str(e)

    def start_process(self, command):
        """Starts a process."""
        try:
            # Running in background
            proc = subprocess.Popen(command, shell=True)
            return f"Started process '{command}' with PID {proc.pid}"
        except Exception as e:
            return f"Failed to start process: {str(e)}"

    def stop_process(self, pid):
        """Stops a process by PID."""
        try:
            os.kill(int(pid), 9) # SIGKILL
            return f"Process {pid} stopped."
        except Exception as e:
            return f"Failed to stop process {pid}: {str(e)}"

    def monitor_process(self, pid):
        """
        Simulates monitoring since standard lib doesn't allow easy 
        cross-platform CPU/Mem reading of specific PIDs without psutil.
        """
        try:
            # Check if process exists
            os.kill(int(pid), 0)
            return f"Process {pid} is RUNNING (CPU/Mem stats require psutil library)"
        except OSError:
            return f"Process {pid} is NOT RUNNING or Access Denied"

def handle_client(conn, addr, pm):
    print(f"[NEW CONNECTION] {addr} connected.")
    connected = True
    while connected:
        try:
            msg = conn.recv(BUFFER_SIZE).decode()
            if not msg:
                break
            
            command_parts = msg.split()
            cmd = command_parts[0].upper()
            
            response = ""
            if cmd == "LIST":
                data = pm.list_processes()
                response = json.dumps(data, indent=2)
            elif cmd == "START":
                if len(command_parts) > 1:
                    response = pm.start_process(" ".join(command_parts[1:]))
                else:
                    response = "Usage: START <command>"
            elif cmd == "STOP":
                if len(command_parts) > 1:
                    response = pm.stop_process(command_parts[1])
                else:
                    response = "Usage: STOP <pid>"
            elif cmd == "MONITOR":
                if len(command_parts) > 1:
                    response = pm.monitor_process(command_parts[1])
                else:
                    response = "Usage: MONITOR <pid>"
            elif cmd == "EXIT":
                response = "Goodbye!"
                connected = False
            else:
                response = "Unknown command. Available: LIST, START, STOP, MONITOR, EXIT"
            
            conn.send(response.encode())
        except Exception as e:
            print(f"Error handling client: {e}")
            break
    
    conn.close()

def start_tcp_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, TCP_PORT))
    server.listen()
    print(f"[TCP SERVER] Listening on {HOST}:{TCP_PORT}")
    
    pm = ProcessManager()
    
    while True:
        conn, addr = server.accept()
        thread = threading.Thread(target=handle_client, args=(conn, addr, pm))
        thread.start()

def start_udp_discovery():
    """Task 3: Service Discovery Listener"""
    udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp.bind(('', UDP_PORT))
    print(f"[UDP DISCOVERY] Listening on port {UDP_PORT}")
    
    while True:
        data, addr = udp.recvfrom(1024)
        msg = data.decode()
        if msg == "DISCOVER_SERVERS":
            # Respond with our TCP port
            response = f"SERVER_AT:{TCP_PORT}"
            udp.sendto(response.encode(), addr)

if __name__ == "__main__":
    print("[STARTING] Server is starting...")
    
    # Start UDP Discovery in background
    udp_thread = threading.Thread(target=start_udp_discovery, daemon=True)
    udp_thread.start()
    
    # Start TCP Server (Main thread)
    start_tcp_server()
