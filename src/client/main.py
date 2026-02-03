import socket
import sys

# Configuration
UDP_PORT = 5001
BUFFER_SIZE = 4096

def discover_servers():
    """Broadcasts a message to find active servers on the local network."""
    print("[DISCOVERY] Looking for servers...")
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    client.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    client.settimeout(2.0)
    
    servers = []
    try:
        client.sendto(b"DISCOVER_SERVERS", ('<broadcast>', UDP_PORT))
        while True:
            try:
                data, addr = client.recvfrom(1024)
                msg = data.decode()
                if msg.startswith("SERVER_AT:"):
                    port = int(msg.split(":")[1])
                    servers.append((addr[0], port))
                    print(f"[FOUND] Server at {addr[0]}:{port}")
            except socket.timeout:
                break
    except Exception as e:
        print(f"Discovery error: {e}")
    
    return servers

def connect_to_server(ip, port):
    try:
        client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect((ip, port))
        return client
    except Exception as e:
        print(f"Connection failed: {e}")
        return None

def main():
    print("=== Process Manager Client ===")
    
    # Task 3: Discovery
    servers = discover_servers()
    
    target_ip = "127.0.0.1"
    target_port = 5000
    
    if servers:
        print("\nAvailable Servers:")
        for idx, s in enumerate(servers):
            print(f"{idx + 1}. {s[0]}:{s[1]}")
        
        choice = input("\nSelect server (default local): ")
        if choice.isdigit() and 1 <= int(choice) <= len(servers):
            target_ip, target_port = servers[int(choice)-1]
    else:
        print("No servers found via discovery.")
    
    manual = input("\nEnter server IP manually? (y/N): ")
    if manual.lower() == 'y':
        target_ip = input("Enter Server IP: ")
        target_port = int(input("Enter Server Port (default 5000): ") or 5000)
    elif not servers:
        print("Using default localhost:5000")

    client = connect_to_server(target_ip, target_port)
    if not client:
        return

    print(f"\nConnected to {target_ip}:{target_port}")
    print("Commands: LIST, START <cmd>, STOP <pid>, MONITOR <pid>, EXIT")

    while True:
        try:
            cmd = input("> ")
            if not cmd:
                continue
            
            client.send(cmd.encode())
            
            if cmd.upper() == "EXIT":
                break
                
            response = client.recv(BUFFER_SIZE).decode()
            print(f"Server response:\n{response}")
            
        except KeyboardInterrupt:
            print("\nExiting...")
            break
        except Exception as e:
            print(f"Error: {e}")
            break

    client.close()

if __name__ == "__main__":
    main()
