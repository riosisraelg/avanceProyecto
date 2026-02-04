#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DEFAULT_PORT 5002
#define BUFFER_SIZE 65536

int connect_to_server(const char *ip, int port) {
    int sock;
    struct sockaddr_in server;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket\n");
        return -1;
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    // Timeout de conexión de 5 segundos
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    return sock;
}

void clean_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    char ip[INET_ADDRSTRLEN];
    int port = DEFAULT_PORT;
    int sock;
    char message[BUFFER_SIZE];
    char server_reply[BUFFER_SIZE];
    char input_buffer[100];

    printf("=== Remote Process Manager Client (Direct Connect) ===\n");
    
    // Configuración de conexión
    printf("Enter Server IP (default 127.0.0.1): ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0; // Remove newline
        if (strlen(input_buffer) > 0) {
            strcpy(ip, input_buffer);
        } else {
            strcpy(ip, "127.0.0.1");
        }
    }

    printf("Enter Port (default %d): ", DEFAULT_PORT);
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) {
            port = atoi(input_buffer);
        }
    }

    printf("\n[CONNECTING] Attempting to connect to %s:%d...\n", ip, port);
    sock = connect_to_server(ip, port);
    
    if (sock < 0) {
        printf("[ERROR] Could not connect to server. Check IP and Firewall rules.\n");
        return 1;
    }

    printf("[SUCCESS] Connected!\n");
    printf("--------------------------------------------------\n");
    printf("Available Commands:\n");
    printf("  LIST          -> List active processes (top 20)\n");
    printf("  START <cmd>   -> Start a new process (e.g., START sleep 10)\n");
    printf("  STOP <pid>    -> Kill a process by PID\n");
    printf("  EXIT          -> Disconnect and exit\n");
    printf("--------------------------------------------------\n");

    while (1) {
        printf("remote@%s> ", ip);
        if (fgets(message, BUFFER_SIZE, stdin) == NULL) break;

        message[strcspn(message, "\n")] = 0; // Remove newline

        if (strlen(message) == 0) continue;

        if (send(sock, message, strlen(message), 0) < 0) {
            puts("Send failed");
            break;
        }

        if (strcmp(message, "EXIT") == 0) break;

        // Recibir respuesta
        int len = recv(sock, server_reply, BUFFER_SIZE - 1, 0);
        if (len < 0) {
            puts("Server closed connection");
            break;
        }
        server_reply[len] = '\0';
        printf("%s\n", server_reply);
    }

    close(sock);
    printf("Disconnected.\n");
    return 0;
}