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
#include <ctype.h>

#define TCP_PORT 5002
#define BUFFER_SIZE 65536

// Función para listar procesos (estilo ps)
void list_processes(char *buffer, size_t size) {
    FILE *fp;
    char path[1035];

    // Ejecuta 'ps' para obtener PID y comando
    fp = popen("ps -e -o pid,comm", "r");
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
    // Validar que pid_str no sea NULL o vacío
    if (pid_str == NULL || strlen(pid_str) == 0) {
        snprintf(buffer, size, "Error: PID no especificado.\n");
        return;
    }

    // Validar que pid_str contenga solo dígitos
    for (int i = 0; pid_str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)pid_str[i])) {
            snprintf(buffer, size, "Error: PID invalido '%s'. Debe ser un numero.\n", pid_str);
            return;
        }
    }

    int pid = atoi(pid_str);
    
    // Validar rango de PID
    if (pid <= 0) {
        snprintf(buffer, size, "Error: PID invalido %d. Debe ser mayor que 0.\n", pid);
        return;
    }

    // Proteger procesos críticos del sistema
    if (pid == 1) {
        snprintf(buffer, size, "Error: No se puede detener el proceso init (PID 1).\n");
        return;
    }

    // Verificar si el proceso existe antes de intentar matarlo
    if (kill(pid, 0) == -1) {
        if (errno == ESRCH) {
            snprintf(buffer, size, "Error: El proceso %d no existe.\n", pid);
        } else if (errno == EPERM) {
            snprintf(buffer, size, "Error: Sin permisos para detener el proceso %d.\n", pid);
        } else {
            snprintf(buffer, size, "Error: No se puede acceder al proceso %d: %s\n", 
                     pid, strerror(errno));
        }
        return;
    }

    // Intentar detener el proceso
    if (kill(pid, SIGKILL) == 0) {
        snprintf(buffer, size, "Proceso %d detenido exitosamente.\n", pid);
    } else {
        if (errno == EPERM) {
            snprintf(buffer, size, "Error: Sin permisos para detener el proceso %d.\n", pid);
        } else {
            snprintf(buffer, size, "Error al detener proceso %d: %s\n", pid, strerror(errno));
        }
    }
}

// Función para iniciar un proceso en segundo plano
void start_process(char *command, char *buffer, size_t size) {
    // Validar que el comando no sea NULL o vacío
    if (command == NULL || strlen(command) == 0) {
        snprintf(buffer, size, "Error: Comando vacio.\n");
        return;
    }

    // Validar longitud del comando
    if (strlen(command) > 1024) {
        snprintf(buffer, size, "Error: Comando demasiado largo (max 1024 caracteres).\n");
        return;
    }

    // Lista de comandos peligrosos bloqueados
    const char *blocked_cmds[] = {
        "rm -rf /", "mkfs", "dd if=/dev/zero", ":(){ :|:& };:", 
        "chmod -R 777 /", "chown -R", "> /dev/sda", "mv / ", NULL
    };

    // Verificar comandos bloqueados
    for (int i = 0; blocked_cmds[i] != NULL; i++) {
        if (strstr(command, blocked_cmds[i]) != NULL) {
            snprintf(buffer, size, "Error: Comando bloqueado por seguridad.\n");
            return;
        }
    }

    pid_t pid = fork();

    if (pid < 0) {
        snprintf(buffer, size, "Error: fork() fallo: %s\n", strerror(errno));
    } else if (pid == 0) {
        // Proceso hijo
        char *args[16];
        int i = 0;
        
        // Crear copia del comando para tokenizar
        char cmd_copy[1024];
        strncpy(cmd_copy, command, sizeof(cmd_copy) - 1);
        cmd_copy[sizeof(cmd_copy) - 1] = '\0';
        
        char *token = strtok(cmd_copy, " ");
        while (token != NULL && i < 15) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (i == 0) {
            exit(1); // No hay comando
        }

        // Redirigir stdout/stderr a /dev/null
        freopen("/dev/null", "w", stdout);
        freopen("/dev/null", "w", stderr);

        execvp(args[0], args);
        
        // Si execvp retorna, hubo un error
        exit(1); 
    } else {
        // Proceso padre
        // Esperar un momento para verificar si el proceso se inició correctamente
        usleep(100000); // 100ms
        
        // Verificar si el proceso hijo sigue vivo
        int status;
        pid_t result = waitpid(pid, &status, WNOHANG);
        
        if (result == 0) {
            // Proceso sigue corriendo
            snprintf(buffer, size, "Proceso '%s' iniciado con PID %d\n", command, pid);
        } else if (result == pid) {
            // Proceso terminó inmediatamente
            if (WIFEXITED(status)) {
                snprintf(buffer, size, 
                         "Error: El proceso termino inmediatamente (codigo %d).\n"
                         "Verifica que el comando '%s' sea valido.\n", 
                         WEXITSTATUS(status), command);
            } else {
                snprintf(buffer, size, 
                         "Error: El proceso termino inesperadamente.\n"
                         "Verifica que el comando '%s' sea valido.\n", command);
            }
        } else {
            // Error en waitpid, pero asumimos que el proceso se inició
            snprintf(buffer, size, "Proceso '%s' iniciado con PID %d\n", command, pid);
        }
    }
}

// Función para normalizar comandos (convertir a mayúsculas y detectar sinónimos)
void normalize_command(char *cmd, char *normalized, size_t size) {
    if (cmd == NULL || normalized == NULL) {
        normalized[0] = '\0';
        return;
    }

    // Convertir a mayúsculas
    char upper[256];
    int i;
    for (i = 0; cmd[i] && i < 255; i++) {
        upper[i] = toupper((unsigned char)cmd[i]);
    }
    upper[i] = '\0';

    // Detectar sinónimos de LIST (listar procesos)
    if (strcmp(upper, "LIST") == 0 || strcmp(upper, "LISTAR") == 0 ||
        strcmp(upper, "LS") == 0 || strcmp(upper, "PS") == 0 ||
        strcmp(upper, "LISTA") == 0 || strcmp(upper, "VER") == 0 ||
        strcmp(upper, "SHOW") == 0 || strcmp(upper, "MOSTRAR") == 0 ||
        strcmp(upper, "PROCESOS") == 0 || strcmp(upper, "PROCESSES") == 0) {
        strncpy(normalized, "LIST", size - 1);
        normalized[size - 1] = '\0';
        return;
    }

    // Detectar sinónimos de START (iniciar proceso)
    if (strcmp(upper, "START") == 0 || strcmp(upper, "INICIAR") == 0 ||
        strcmp(upper, "RUN") == 0 || strcmp(upper, "EJECUTAR") == 0 ||
        strcmp(upper, "LAUNCH") == 0 || strcmp(upper, "LANZAR") == 0 ||
        strcmp(upper, "EXEC") == 0 || strcmp(upper, "CORRER") == 0 ||
        strcmp(upper, "BEGIN") == 0 || strcmp(upper, "EMPEZAR") == 0 ||
        strcmp(upper, "CREATE") == 0 || strcmp(upper, "CREAR") == 0) {
        strncpy(normalized, "START", size - 1);
        normalized[size - 1] = '\0';
        return;
    }

    // Detectar sinónimos de STOP (detener proceso)
    if (strcmp(upper, "STOP") == 0 || strcmp(upper, "DETENER") == 0 ||
        strcmp(upper, "KILL") == 0 || strcmp(upper, "MATAR") == 0 ||
        strcmp(upper, "END") == 0 || strcmp(upper, "TERMINAR") == 0 ||
        strcmp(upper, "FINISH") == 0 || strcmp(upper, "FINALIZAR") == 0 ||
        strcmp(upper, "CANCEL") == 0 || strcmp(upper, "CANCELAR") == 0 ||
        strcmp(upper, "HALT") == 0 || strcmp(upper, "PARAR") == 0) {
        strncpy(normalized, "STOP", size - 1);
        normalized[size - 1] = '\0';
        return;
    }

    // Detectar sinónimos de EXIT (salir)
    if (strcmp(upper, "EXIT") == 0 || strcmp(upper, "SALIR") == 0 ||
        strcmp(upper, "QUIT") == 0 || strcmp(upper, "BYE") == 0 ||
        strcmp(upper, "ADIOS") == 0 || strcmp(upper, "CLOSE") == 0 ||
        strcmp(upper, "CERRAR") == 0 || strcmp(upper, "DISCONNECT") == 0 ||
        strcmp(upper, "DESCONECTAR") == 0) {
        strncpy(normalized, "EXIT", size - 1);
        normalized[size - 1] = '\0';
        return;
    }

    // Si no coincide con ningún sinónimo, devolver el comando original en mayúsculas
    strncpy(normalized, upper, size - 1);
    normalized[size - 1] = '\0';
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

        // Guardar copia del buffer original para extraer argumentos
        char buffer_copy[BUFFER_SIZE];
        strncpy(buffer_copy, buffer, sizeof(buffer_copy) - 1);
        buffer_copy[sizeof(buffer_copy) - 1] = '\0';

        char *cmd = strtok(buffer, " ");
        char *arg = strtok(NULL, ""); 

        if (cmd == NULL || strlen(cmd) == 0) {
            strcpy(response, "Error: Comando vacio. Usa LIST, START <comando>, STOP <pid>, o EXIT\n");
        } else {
            // Normalizar el comando
            char normalized[64];
            normalize_command(cmd, normalized, sizeof(normalized));

            if (strcmp(normalized, "LIST") == 0) {
                list_processes(response, sizeof(response));
            } else if (strcmp(normalized, "START") == 0) {
                if (arg && strlen(arg) > 0) {
                    start_process(arg, response, sizeof(response));
                } else {
                    strcpy(response, "Error: START requiere un comando.\nEjemplo: START sleep 30\n");
                }
            } else if (strcmp(normalized, "STOP") == 0) {
                if (arg && strlen(arg) > 0) {
                    stop_process(arg, response, sizeof(response));
                } else {
                    strcpy(response, "Error: STOP requiere un PID.\nEjemplo: STOP 1234\n");
                }
            } else if (strcmp(normalized, "EXIT") == 0) {
                strcpy(response, "Adios! Cerrando conexion...\n");
                send(sock, response, strlen(response), 0);
                break;
            } else {
                snprintf(response, sizeof(response), 
                         "Error: Comando desconocido '%s'.\n"
                         "Comandos disponibles:\n"
                         "  LIST/LISTAR - Ver procesos\n"
                         "  START/INICIAR <cmd> - Crear proceso\n"
                         "  STOP/MATAR <pid> - Detener proceso\n"
                         "  EXIT/SALIR - Desconectar\n", cmd);
            }
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