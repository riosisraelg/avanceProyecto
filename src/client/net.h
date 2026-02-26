#ifndef NET_H
#define NET_H

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close_socket closesocket
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define close_socket close
#endif

#define NET_BUFFER_SIZE 65536

/* Connects to the server. Returns socket or INVALID_SOCKET on error. */
SOCKET net_connect(const char *ip, int port);

/* Sends a command string to the server. Returns bytes sent or -1 on error. */
int net_send(SOCKET sock, const char *cmd);

/* Non-blocking receive using select(). Returns bytes read, 0 if no data, -1 on error. */
int net_recv(SOCKET sock, char *buf, int buf_size);

/* Closes the socket connection. */
void net_close(SOCKET sock);

/* Initializes Winsock on Windows. No-op on POSIX. Returns 0 on success. */
int net_init_platform(void);

/* Cleans up Winsock on Windows. No-op on POSIX. */
void net_cleanup_platform(void);

#endif
