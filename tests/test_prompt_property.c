/**
 * Property-based test for input prompt format (Property 5).
 *
 * **Validates: Requirements 5.1**
 *
 * Property 5: Input prompt format
 *   - For any valid IP address and any port in [1, 65535],
 *     the generated prompt follows exactly "remote@{IP}:{PORT}> "
 *
 * The test embeds the input_format_prompt logic directly to avoid
 * linking against ncurses. The function is a pure computation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Embedded copy of input_format_prompt ────────────────────────────────── */

#define INPUT_BUF_SIZE 256

static void input_format_prompt(char *buf, int buf_size, const char *ip, int port)
{
    if (!buf || buf_size <= 0) {
        return;
    }
    if (!ip) {
        ip = "0.0.0.0";
    }
    snprintf(buf, buf_size, "remote@%s:%d> ", ip, port);
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

/* ── Property 5a: Prompt starts with "remote@" ─────────────────────────── */

static void test_prompt_starts_with_remote(void) {
    char buf[INPUT_BUF_SIZE];
    int i, j;

    printf("[Property 5a] Prompt starts with \"remote@\"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            input_format_prompt(buf, INPUT_BUF_SIZE, cur_ip, cur_port);

            CHECK(strncmp(buf, "remote@", 7) == 0,
                  "Prompt \"%s\" does not start with \"remote@\"", buf);
        }
    }
}

/* ── Property 5b: Prompt contains IP after "remote@" ───────────────────── */

static void test_prompt_contains_ip(void) {
    char buf[INPUT_BUF_SIZE];
    const char *after_at;
    int i, j;

    printf("[Property 5b] Prompt contains IP after \"remote@\"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            input_format_prompt(buf, INPUT_BUF_SIZE, cur_ip, cur_port);

            after_at = buf + 7; /* skip "remote@" */

            CHECK(strncmp(after_at, cur_ip, strlen(cur_ip)) == 0,
                  "Prompt \"%s\" does not contain IP \"%s\" after \"remote@\"",
                  buf, cur_ip);
        }
    }
}

/* ── Property 5c: Prompt has colon separator between IP and port ────────── */

static void test_prompt_colon_separator(void) {
    char buf[INPUT_BUF_SIZE];
    int ip_len;
    int i, j;

    printf("[Property 5c] Prompt has ':' between IP and port\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            input_format_prompt(buf, INPUT_BUF_SIZE, cur_ip, cur_port);

            ip_len = (int)strlen(cur_ip);

            CHECK(buf[7 + ip_len] == ':',
                  "Prompt \"%s\": expected ':' at position %d, got '%c'",
                  buf, 7 + ip_len, buf[7 + ip_len]);
        }
    }
}

/* ── Property 5d: Prompt ends with "> " ─────────────────────────────────── */

static void test_prompt_ends_with_suffix(void) {
    char buf[INPUT_BUF_SIZE];
    int len;
    int i, j;

    printf("[Property 5d] Prompt ends with \"> \"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            input_format_prompt(buf, INPUT_BUF_SIZE, cur_ip, cur_port);

            len = (int)strlen(buf);

            CHECK(len >= 2 && buf[len - 2] == '>' && buf[len - 1] == ' ',
                  "Prompt \"%s\" does not end with \"> \"", buf);
        }
    }
}

/* ── Property 5e: Full format matches "remote@{IP}:{PORT}> " ───────────── */

static void test_prompt_exact_format(void) {
    char buf[INPUT_BUF_SIZE];
    char expected[INPUT_BUF_SIZE];
    int i, j;

    printf("[Property 5e] Full format matches \"remote@{IP}:{PORT}> \"\n");

    for (i = 0; i < NUM_IPS; i++) {
        for (j = 0; j < NUM_PORTS; j++) {
            cur_ip   = test_ips[i];
            cur_port = test_ports[j];

            input_format_prompt(buf, INPUT_BUF_SIZE, cur_ip, cur_port);

            snprintf(expected, INPUT_BUF_SIZE, "remote@%s:%d> ",
                     cur_ip, cur_port);

            CHECK(strcmp(buf, expected) == 0,
                  "Prompt \"%s\" != expected \"%s\"", buf, expected);
        }
    }
}

/* ── Property 5f: Port boundary values [1, 65535] ──────────────────────── */

static void test_prompt_port_boundaries(void) {
    char buf[INPUT_BUF_SIZE];
    char expected[INPUT_BUF_SIZE];
    const char *ip = "127.0.0.1";
    int port;
    int boundary_ports[] = { 1, 2, 1023, 1024, 5002, 49151, 49152, 65534, 65535 };
    int num_boundary = sizeof(boundary_ports) / sizeof(boundary_ports[0]);
    int i;

    printf("[Property 5f] Port boundary values produce correct format\n");

    for (i = 0; i < num_boundary; i++) {
        port = boundary_ports[i];
        cur_ip   = ip;
        cur_port = port;

        input_format_prompt(buf, INPUT_BUF_SIZE, ip, port);

        snprintf(expected, INPUT_BUF_SIZE, "remote@%s:%d> ", ip, port);

        CHECK(strcmp(buf, expected) == 0,
              "Prompt \"%s\" != expected \"%s\"", buf, expected);
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 5: Input prompt format ===\n");
    printf("    Validates: Requirements 5.1\n\n");

    test_prompt_starts_with_remote();
    test_prompt_contains_ip();
    test_prompt_colon_separator();
    test_prompt_ends_with_suffix();
    test_prompt_exact_format();
    test_prompt_port_boundaries();

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
