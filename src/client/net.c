#include "net.h"

#include <stdio.h>
#include <string.h>

int net_init_platform(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }
#endif
    return 0;
}

void net_cleanup_platform(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

SOCKET net_connect(const char *ip, int port) {
    SOCKET sock;
    struct sockaddr_in server;

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

int net_send(SOCKET sock, const char *cmd) {
    if (sock == INVALID_SOCKET || cmd == NULL) {
        return -1;
    }
    int len = (int)strlen(cmd);
    int sent = send(sock, cmd, len, 0);
    if (sent < 0) {
        return -1;
    }
    return sent;
}

int net_recv(SOCKET sock, char *buf, int buf_size) {
    if (sock == INVALID_SOCKET || buf == NULL || buf_size <= 0) {
        return -1;
    }

    fd_set read_fds;
    struct timeval tv;

    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);

    /* Zero timeout for non-blocking check */
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    int ready = select((int)sock + 1, &read_fds, NULL, NULL, &tv);
    if (ready < 0) {
        return -1;
    }
    if (ready == 0) {
        return 0; /* No data available */
    }

    int received = recv(sock, buf, buf_size - 1, 0);
    if (received < 0) {
        return -1;
    }
    if (received == 0) {
        return -1; /* Connection closed by peer */
    }

    buf[received] = '\0';
    return received;
}

void net_close(SOCKET sock) {
    if (sock != INVALID_SOCKET) {
        close_socket(sock);
    }
}
