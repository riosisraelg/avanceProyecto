/**
 * Unit tests for process_list_parse() and process_list_free().
 *
 * Tests the pure parsing logic without ncurses dependency.
 * Validates: Requirements 4.1, 4.3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── Embedded types and parse logic (no ncurses dependency) ─────────── */

#define PROC_NAME_SIZE 256
#define INITIAL_CAPACITY 32

typedef struct {
    int pid;
    char name[PROC_NAME_SIZE];
} ProcessEntry;

typedef struct {
    ProcessEntry *entries;
    int count;
    int capacity;
} ProcessList;

int process_list_parse(const char *raw_response, ProcessList *list)
{
    const char *line_start;
    const char *p;
    int is_first_line;

    if (!list)
        return -1;

    list->entries  = NULL;
    list->count    = 0;
    list->capacity = 0;

    if (!raw_response || raw_response[0] == '\0')
        return 0;

    list->entries = malloc(INITIAL_CAPACITY * sizeof(ProcessEntry));
    if (!list->entries)
        return -1;
    list->capacity = INITIAL_CAPACITY;

    is_first_line = 1;
    line_start = raw_response;

    while (*line_start != '\0') {
        p = line_start;
        while (*p != '\0' && *p != '\n')
            p++;

        int line_len = (int)(p - line_start);

        if (is_first_line) {
            is_first_line = 0;
        } else if (line_len > 0) {
            const char *s = line_start;

            while (s < line_start + line_len && isspace((unsigned char)*s))
                s++;

            if (s < line_start + line_len) {
                int pid = 0;
                int has_digit = 0;
                while (s < line_start + line_len && isdigit((unsigned char)*s)) {
                    pid = pid * 10 + (*s - '0');
                    has_digit = 1;
                    s++;
                }

                if (has_digit) {
                    while (s < line_start + line_len && isspace((unsigned char)*s))
                        s++;

                    int name_len = (int)(line_start + line_len - s);

                    if (list->count >= list->capacity) {
                        int new_cap = list->capacity * 2;
                        ProcessEntry *tmp = realloc(list->entries,
                                                    (size_t)new_cap * sizeof(ProcessEntry));
                        if (!tmp)
                            return -1;
                        list->entries  = tmp;
                        list->capacity = new_cap;
                    }

                    ProcessEntry *entry = &list->entries[list->count];
                    entry->pid = pid;

                    if (name_len > 0 && name_len < PROC_NAME_SIZE) {
                        memcpy(entry->name, s, (size_t)name_len);
                        entry->name[name_len] = '\0';
                    } else if (name_len >= PROC_NAME_SIZE) {
                        memcpy(entry->name, s, PROC_NAME_SIZE - 1);
                        entry->name[PROC_NAME_SIZE - 1] = '\0';
                    } else {
                        entry->name[0] = '\0';
                    }

                    list->count++;
                }
            }
        }

        if (*p == '\n')
            line_start = p + 1;
        else
            break;
    }

    return 0;
}

void process_list_free(ProcessList *list)
{
    if (!list)
        return;
    if (list->entries) {
        free(list->entries);
        list->entries = NULL;
    }
    list->count    = 0;
    list->capacity = 0;
}

/* ── Test helpers ───────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            tests_failed++;                                         \
            fprintf(stderr, "  FAIL: " fmt "\n", ##__VA_ARGS__);    \
        }                                                           \
    } while (0)

/* ── Test: Typical ps output ────────────────────────────────────────── */

static void test_typical_ps_output(void) {
    const char *input =
        "  PID COMM\n"
        "    1 init\n"
        "  234 nginx\n"
        " 5678 node\n";

    ProcessList list;
    int rc = process_list_parse(input, &list);

    printf("[Test] Typical ps output\n");
    CHECK(rc == 0, "parse returned %d, expected 0", rc);
    CHECK(list.count == 3, "count=%d, expected 3", list.count);

    if (list.count >= 3) {
        CHECK(list.entries[0].pid == 1, "pid[0]=%d, expected 1", list.entries[0].pid);
        CHECK(strcmp(list.entries[0].name, "init") == 0,
              "name[0]='%s', expected 'init'", list.entries[0].name);

        CHECK(list.entries[1].pid == 234, "pid[1]=%d, expected 234", list.entries[1].pid);
        CHECK(strcmp(list.entries[1].name, "nginx") == 0,
              "name[1]='%s', expected 'nginx'", list.entries[1].name);

        CHECK(list.entries[2].pid == 5678, "pid[2]=%d, expected 5678", list.entries[2].pid);
        CHECK(strcmp(list.entries[2].name, "node") == 0,
              "name[2]='%s', expected 'node'", list.entries[2].name);
    }

    process_list_free(&list);
}

/* ── Test: Empty response ───────────────────────────────────────────── */

static void test_empty_response(void) {
    ProcessList list;

    printf("[Test] Empty response\n");

    int rc = process_list_parse("", &list);
    CHECK(rc == 0, "parse returned %d, expected 0", rc);
    CHECK(list.count == 0, "count=%d, expected 0", list.count);

    process_list_free(&list);
}

/* ── Test: NULL response ────────────────────────────────────────────── */

static void test_null_response(void) {
    ProcessList list;

    printf("[Test] NULL response\n");

    int rc = process_list_parse(NULL, &list);
    CHECK(rc == 0, "parse returned %d, expected 0", rc);
    CHECK(list.count == 0, "count=%d, expected 0", list.count);

    process_list_free(&list);
}

/* ── Test: Header only (no data lines) ──────────────────────────────── */

static void test_header_only(void) {
    const char *input = "  PID COMM\n";
    ProcessList list;

    printf("[Test] Header only\n");

    int rc = process_list_parse(input, &list);
    CHECK(rc == 0, "parse returned %d, expected 0", rc);
    CHECK(list.count == 0, "count=%d, expected 0", list.count);

    process_list_free(&list);
}

/* ── Test: No trailing newline ──────────────────────────────────────── */

static void test_no_trailing_newline(void) {
    const char *input =
        "  PID COMM\n"
        "  100 bash";

    ProcessList list;

    printf("[Test] No trailing newline\n");

    int rc = process_list_parse(input, &list);
    CHECK(rc == 0, "parse returned %d, expected 0", rc);
    CHECK(list.count == 1, "count=%d, expected 1", list.count);

    if (list.count >= 1) {
        CHECK(list.entries[0].pid == 100, "pid=%d, expected 100", list.entries[0].pid);
        CHECK(strcmp(list.entries[0].name, "bash") == 0,
              "name='%s', expected 'bash'", list.entries[0].name);
    }

    process_list_free(&list);
}

/* ── Test: Free clears fields ───────────────────────────────────────── */

static void test_free_clears_fields(void) {
    const char *input =
        "  PID COMM\n"
        "    1 init\n";

    ProcessList list;

    printf("[Test] Free clears fields\n");

    process_list_parse(input, &list);
    CHECK(list.count == 1, "count=%d before free, expected 1", list.count);

    process_list_free(&list);
    CHECK(list.entries == NULL, "entries not NULL after free");
    CHECK(list.count == 0, "count=%d after free, expected 0", list.count);
    CHECK(list.capacity == 0, "capacity=%d after free, expected 0", list.capacity);
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Unit Tests: process_list_parse / process_list_free ===\n");
    printf("    Validates: Requirements 4.1, 4.3\n\n");

    test_typical_ps_output();
    test_empty_response();
    test_null_response();
    test_header_only();
    test_no_trailing_newline();
    test_free_clears_fields();

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
