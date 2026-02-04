#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>

#define TCP_PORT 5002
#define BUFFER_SIZE 4096

// Función para listar procesos (estilo ps)
void list_processes(char *buffer, size_t size) {
    FILE *fp;
    char path[1035];

    // Ejecuta 'ps' para obtener PID y comando
    fp = popen("ps -e -o pid,comm | head -n 20", "r");
    if (fp == NULL) {
        snprintf(buffer, size, "Error: Failed to run ps command\n");
        return;
    }

    strcpy(buffer, "");
    while (fgets(path, sizeof(path), fp) != NULL) {
        if (strlen(buffer) + strlen(path) < size - 1) {
            strcat(buffer, path);
        } else {
            break;
        }
    }
    pclose(fp);
}

// Función para detener un proceso
void stop_process(char *pid_str, char *buffer, size_t size) {
    int pid = atoi(pid_str);
    if (pid <= 0) {
        snprintf(buffer, size, "Invalid PID\n");
        return;
    }

    if (kill(pid, SIGKILL) == 0) {
        snprintf(buffer, size, "Process %d stopped successfully.\n", pid);
    } else {
        snprintf(buffer, size, "Failed to stop process %d: %s\n", pid, strerror(errno));
    }
}

// Función para iniciar un proceso en segundo plano
void start_process(char *command, char *buffer, size_t size) {
    pid_t pid = fork();

    if (pid < 0) {
        snprintf(buffer, size, "Fork failed\n");
    } else if (pid == 0) {
        // Proceso hijo
        char *args[16];
        int i = 0;
        char *token = strtok(command, " ");
        while (token != NULL && i < 15) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        // Redirigir stdout/stderr a /dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        execvp(args[0], args);
        exit(1); 
    } else {
        // Proceso padre
        snprintf(buffer, size, "Started process '%s' with PID %d\n", command, pid);
    }
}

// Manejador del cliente TCP
void *handle_client(void *socket_desc) {
    int sock = *(int*)socket_desc;
    free(socket_desc);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    int read_size;

    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(sock, (struct sockaddr*)&addr, &addr_size);
    printf("[TCP] Connection from %s\n", inet_ntoa(addr.sin_addr));

    while ((read_size = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[read_size] = '\0';
        buffer[strcspn(buffer, "\r\n")] = 0;

        printf("[CMD from %s]: %s\n", inet_ntoa(addr.sin_addr), buffer);

        char *cmd = strtok(buffer, " ");
        char *arg = strtok(NULL, ""); 

        if (cmd == NULL) {
            strcpy(response, "Empty command\n");
        } else if (strcmp(cmd, "LIST") == 0) {
            list_processes(response, sizeof(response));
        } else if (strcmp(cmd, "START") == 0) {
            if (arg) start_process(arg, response, sizeof(response));
            else strcpy(response, "Usage: START <command>\n");
        } else if (strcmp(cmd, "STOP") == 0) {
            if (arg) stop_process(arg, response, sizeof(response));
            else strcpy(response, "Usage: STOP <pid>\n");
        } else if (strcmp(cmd, "EXIT") == 0) {
            strcpy(response, "Goodbye!\n");
            send(sock, response, strlen(response), 0);
            break;
        } else {
            strcpy(response, "Unknown command\n");
        }

        send(sock, response, strlen(response), 0);
    }

    close(sock);
    printf("[TCP] Client %s disconnected\n", inet_ntoa(addr.sin_addr));
    return NULL;
}

// Limpiar procesos zombies
void sigchld_handler(int s) {
    (void)s;
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int server_sock, *new_sock;
    struct sockaddr_in server, client;
    socklen_t c = sizeof(struct sockaddr_in);

    struct sigaction sa;
    sa.sa_handler = sigchld_handler; 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        printf("Could not create TCP socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(TCP_PORT);

    if (bind(server_sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("TCP Bind failed");
        return 1;
    }

    listen(server_sock, 3);
    printf("=== Process Manager Server (TCP Only) ===\n");
    printf("[INFO] Listening on 0.0.0.0:%d\n", TCP_PORT);
    printf("[INFO] Ready for external connections...\n");

    while (1) {
        int client_sock = accept(server_sock, (struct sockaddr *)&client, &c);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        pthread_t tcp_thread;
        new_sock = malloc(sizeof(int));
        *new_sock = client_sock;
        
        if (pthread_create(&tcp_thread, NULL, handle_client, (void*)new_sock) < 0) {
            perror("Could not create TCP thread");
            free(new_sock);
            continue;
        }
        
        pthread_detach(tcp_thread);
    }

    return 0;
}