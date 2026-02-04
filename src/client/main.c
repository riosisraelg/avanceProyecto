#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close_socket closesocket
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #define close_socket close
    typedef int SOCKET;
    #define INVALID_SOCKET -1
#endif

#define DEFAULT_PORT 5002
#define BUFFER_SIZE 65536

void to_uppercase(char *str) {
    for (int i = 0; str[i]; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

SOCKET connect_to_server(const char *ip, int port) {
    SOCKET sock;
    struct sockaddr_in server;

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return INVALID_SOCKET;
    }
#endif

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Could not create socket\n");
        return INVALID_SOCKET;
    }

    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

#ifndef _WIN32
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close_socket(sock);
        return INVALID_SOCKET;
    }

    return sock;
}

int main() {
    char ip[INET_ADDRSTRLEN];
    int port = DEFAULT_PORT;
    SOCKET sock;
    char message[BUFFER_SIZE];
    char final_msg[BUFFER_SIZE];
    char server_reply[BUFFER_SIZE];
    char input_buffer[100];

    printf("=== Remote Process Manager Client (Cross-Platform) ===\n");
    
    printf("Enter Server IP (default 127.0.0.1): ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) strcpy(ip, input_buffer);
        else strcpy(ip, "127.0.0.1");
    }

    printf("Enter Port (default %d): ", DEFAULT_PORT);
    if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) > 0) port = atoi(input_buffer);
    }

    printf("\n[CONNECTING] Attempting to connect to %s:%d...\n", ip, port);
    sock = connect_to_server(ip, port);
    
    if (sock == INVALID_SOCKET) {
        printf("[ERROR] Could not connect to server.\n");
        return 1;
    }

    printf("[SUCCESS] Connected!\n");
    printf("--------------------------------------------------\n");
    printf("Available Commands:\n");
    printf("  1. LIST          -> List all active processes\n");
    printf("  2. START <cmd>   -> Start a new process\n");
    printf("  3. STOP <pid>    -> Kill a process by PID\n");
    printf("  4. EXIT          -> Disconnect and exit\n");
    printf("--------------------------------------------------\n");

    while (1) {
        printf("remote@%s:%d> ", ip, port);
        if (fgets(message, BUFFER_SIZE, stdin) == NULL) break;
        message[strcspn(message, "\n")] = 0;

        if (strlen(message) == 0) continue;

        char temp_msg[BUFFER_SIZE];
        strcpy(temp_msg, message);
        char *cmd_part = strtok(temp_msg, " ");
        char *args_part = strtok(NULL, "");

        if (cmd_part == NULL) continue;

        char normalized_cmd[32];
        strncpy(normalized_cmd, cmd_part, 31);
        normalized_cmd[31] = '\0';
        to_uppercase(normalized_cmd);

        if (strcmp(normalized_cmd, "1") == 0) strcpy(normalized_cmd, "LIST");
        else if (strcmp(normalized_cmd, "2") == 0) strcpy(normalized_cmd, "START");
        else if (strcmp(normalized_cmd, "3") == 0) strcpy(normalized_cmd, "STOP");
        else if (strcmp(normalized_cmd, "4") == 0) strcpy(normalized_cmd, "EXIT");

        if (args_part) {
            snprintf(final_msg, BUFFER_SIZE, "%s %s", normalized_cmd, args_part);
        } else {
            snprintf(final_msg, BUFFER_SIZE, "%s", normalized_cmd);
        }

        if (send(sock, final_msg, (int)strlen(final_msg), 0) < 0) {
            puts("Send failed");
            break;
        }

        if (strcmp(normalized_cmd, "EXIT") == 0) break;

        int len = recv(sock, server_reply, BUFFER_SIZE - 1, 0);
        if (len <= 0) {
            puts("Server closed connection");
            break;
        }
        server_reply[len] = '\0';
        printf("%s\n", server_reply);
    }

    close_socket(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}