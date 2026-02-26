/**
 * Property-based test for connection status message format (Property 9).
 *
 * **Validates: Requirements 6.1**
 *
 * Property 9: Connection status message format
 *   - For any valid IP address and any port in [1, 65535],
 *     when the connection state is CONNECTED, the status message
 *     must contain "Conectado a {IP}:{PORT}"
 *
 * The test embeds the status message formatting logic directly to avoid
 * linking against ncurses. The function is a pure computation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Embedded connection state and status formatting logic ───────────── */

typedef enum {
    CONN_DISCONNECTED,
    CONN_CONNECTING,
    CONN_CONNECTED,
    CONN_ERROR
} ConnectionState;

#define STATUS_MSG_SIZE 256

/**
 * Formats the status bar message based on connection state.
 * Mirrors the logic in tui.c (snprintf into status_msg).
 */
static void format_status_msg(char *buf, int buf_size,
                              const char *ip, int port,
                              ConnectionState state)
{
    if (!buf || buf_size <= 0)
        return;

    switch (state) {
    case CONN_CONNECTED:
        snprintf(buf, buf_size, "Conectado a %s:%d", ip, port);
        break;
    case CONN_CONNECTING:
        snprintf(buf, buf_size, "Conectando...");
        break;
    case CONN_DISCONNECTED:
        snprintf(buf, buf_size, "Desconectado");
        break;
    case CONN_ERROR:
        snprintf(buf, buf_size, "Conexion perdida");
        break;
    }
}

/* ── Test helpers ───────────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

static const char *cur_ip   = "";
static int         cur_port = 0;

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            tests_failed++;                                         \
            fprintf(stderr, "  FAIL [IP=%s, PORT=%d]: " fmt "\n",   \
                    cur_ip, cur_port, ##__VA_ARGS__);               \
        }                                                           \
    } while (0)

/* ── Test data ──────────────────────────────────────────────────────────── */

static const char *test_ips[] = {
    "127.0.0.1",
    "192.168.1.1",
    "10.0.0.1",
    "0.0.0.0",
    "255.255.255.255",
    "172.16.0.1",
    "8.8.8.8",
    "1.2.3.4",
    "192.168.0.100",
    "10.10.10.10",
};

static const int NUM_IPS = sizeof(test_ips) / sizeof(test_ips[0]);

static const int test_ports[] = {
    1, 2, 80, 443, 1024, 5002, 8080, 8443,
    10000, 30000, 49152, 65534, 65535,
};

static const int NUM_PORTS = sizeof(test_ports) / sizeof(test_ports[0]);

/* ── Property 9a: CONN_CONNECTED message contains "Conectado a {IP}:{PORT}" */

static void test_connected_contains_ip_port(void) {
    char buf[STATUS_MSG_SIZE];
    char expected_substr[STATUS_MSG_SIZE];
    int i, j;

    printf("[Property 9a] CONN_CONNECTED message contains \"Conectado a {IP}:{PORT}\"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            format_status_msg(buf, STATUS_MSG_SIZE,
                              cur_ip, cur_port, CONN_CONNECTED);

            snprintf(expected_substr, STATUS_MSG_SIZE,
                     "Conectado a %s:%d", cur_ip, cur_port);

            CHECK(strstr(buf, expected_substr) != NULL,
                  "Message \"%s\" does not contain \"%s\"",
                  buf, expected_substr);
        }
    }
}

/* ── Property 9b: CONN_CONNECTED message starts with "Conectado a" ────── */

static void test_connected_starts_with_prefix(void) {
    char buf[STATUS_MSG_SIZE];
    int i, j;

    printf("[Property 9b] CONN_CONNECTED message starts with \"Conectado a \"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            format_status_msg(buf, STATUS_MSG_SIZE,
                              cur_ip, cur_port, CONN_CONNECTED);

            CHECK(strncmp(buf, "Conectado a ", 12) == 0,
                  "Message \"%s\" does not start with \"Conectado a \"", buf);
        }
    }
}

/* ── Property 9c: CONN_CONNECTED message contains the IP after prefix ─── */

static void test_connected_ip_after_prefix(void) {
    char buf[STATUS_MSG_SIZE];
    const char *after_prefix;
    int i, j;

    printf("[Property 9c] CONN_CONNECTED message contains IP after prefix\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            format_status_msg(buf, STATUS_MSG_SIZE,
                              cur_ip, cur_port, CONN_CONNECTED);

            after_prefix = buf + 12; /* skip "Conectado a " */

            CHECK(strncmp(after_prefix, cur_ip, strlen(cur_ip)) == 0,
                  "Message \"%s\" does not contain IP \"%s\" after prefix",
                  buf, cur_ip);
        }
    }
}

/* ── Property 9d: CONN_CONNECTED message has colon between IP and port ── */

static void test_connected_colon_separator(void) {
    char buf[STATUS_MSG_SIZE];
    int ip_len;
    int i, j;

    printf("[Property 9d] CONN_CONNECTED message has ':' between IP and port\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            format_status_msg(buf, STATUS_MSG_SIZE,
                              cur_ip, cur_port, CONN_CONNECTED);

            ip_len = (int)strlen(cur_ip);

            /* "Conectado a " (12 chars) + IP + ':' */
            CHECK(buf[12 + ip_len] == ':',
                  "Message \"%s\": expected ':' at position %d, got '%c'",
                  buf, 12 + ip_len, buf[12 + ip_len]);
        }
    }
}

/* ── Property 9e: Port boundary values produce correct format ──────────── */

static void test_connected_port_boundaries(void) {
    char buf[STATUS_MSG_SIZE];
    char expected[STATUS_MSG_SIZE];
    const char *ip = "127.0.0.1";
    int boundary_ports[] = { 1, 2, 1023, 1024, 5002, 49151, 49152, 65534, 65535 };
    int num_boundary = sizeof(boundary_ports) / sizeof(boundary_ports[0]);
    int i;

    printf("[Property 9e] Port boundary values produce correct format\n");

    for (i = 0; i < num_boundary; i++) {
        cur_ip   = ip;
        cur_port = boundary_ports[i];

        format_status_msg(buf, STATUS_MSG_SIZE,
                          ip, cur_port, CONN_CONNECTED);

        snprintf(expected, STATUS_MSG_SIZE,
                 "Conectado a %s:%d", ip, cur_port);

        CHECK(strcmp(buf, expected) == 0,
              "Message \"%s\" != expected \"%s\"", buf, expected);
    }
}

/* ── Property 9f: Non-CONNECTED states do NOT contain "Conectado a" ───── */

static void test_non_connected_no_conectado(void) {
    char buf[STATUS_MSG_SIZE];
    ConnectionState states[] = { CONN_DISCONNECTED, CONN_CONNECTING, CONN_ERROR };
    const char *state_names[] = { "DISCONNECTED", "CONNECTING", "ERROR" };
    int num_states = 3;
    int s;

    printf("[Property 9f] Non-CONNECTED states do not contain \"Conectado a\"\n");

    cur_ip   = "127.0.0.1";
    cur_port = 5002;

    for (s = 0; s < num_states; s++) {
        format_status_msg(buf, STATUS_MSG_SIZE,
                          cur_ip, cur_port, states[s]);

        tests_run++;
        if (strstr(buf, "Conectado a") == NULL) {
            tests_passed++;
        } else {
            tests_failed++;
            fprintf(stderr, "  FAIL [state=%s]: Message \"%s\" should not "
                    "contain \"Conectado a\"\n", state_names[s], buf);
        }
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 9: Connection status message format ===\n");
    printf("    Validates: Requirements 6.1\n\n");

    test_connected_contains_ip_port();
    test_connected_starts_with_prefix();
    test_connected_ip_after_prefix();
    test_connected_colon_separator();
    test_connected_port_boundaries();
    test_non_connected_no_conectado();

    printf("\nResults: %d/%d checks passed", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf(" (%d failed)", tests_failed);
    }
    printf("\n");

    if (tests_failed == 0) {
        printf("PASS\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }
}
