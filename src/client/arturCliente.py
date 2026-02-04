import socket

SERVER_IP = "10.73.135.86"  # ← IP REAL DEL SERVIDOR
PORT = 5007

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((SERVER_IP, PORT))

print("Conectado al servidor")
print("Comandos disponibles: LIST | INFO <pid> | KILL <pid> | EXIT")

while True:
    comando = input(">> ")

    client.sendall(comando.encode())

    respuesta = client.recv(8192).decode()
    print(respuesta)

    if comando.upper() == "EXIT":
        break

client.close()
