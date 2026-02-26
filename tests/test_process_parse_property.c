/**
 * Property-based test for process_list_parse() (Property 3).
 *
 * **Validates: Requirements 4.1**
 *
 * Property 3: Parseo de lista de procesos
 *   - For any text with `ps -e -o pid,comm` format (header line followed
 *     by data lines with numeric PID and name), process_list_parse must:
 *       (a) Extract each PID as the correct integer
 *       (b) Extract each name as the correct string
 *       (c) Return count == number of data lines (excluding header)
 *
 * The test embeds the parse logic directly to avoid linking against ncurses.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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

static int cur_iter = 0;  /* Current iteration for CHECK context */

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            tests_failed++;                                         \
            fprintf(stderr, "  FAIL [iter=%d]: " fmt "\n",          \
                    cur_iter, ##__VA_ARGS__);                        \
        }                                                           \
    } while (0)

/* ── Random generators ──────────────────────────────────────────────── */

/* Random int in [lo, hi] inclusive */
static int rand_range(int lo, int hi)
{
    return lo + rand() % (hi - lo + 1);
}

/* Generate a random process name (lowercase letters, 1..max_len chars) */
static void rand_proc_name(char *buf, int max_len)
{
    int len = rand_range(1, max_len);
    int i;
    for (i = 0; i < len; i++)
        buf[i] = 'a' + (rand() % 26);
    buf[len] = '\0';
}

/* ── Build a random ps-formatted string with known PIDs and names ──── */

#define MAX_TEST_PROCS 50
#define MAX_NAME_LEN   30

typedef struct {
    int pids[MAX_TEST_PROCS];
    char names[MAX_TEST_PROCS][MAX_NAME_LEN + 1];
    int num_procs;
    char *text;  /* The generated ps output string */
} GeneratedInput;

/*
 * Generates a random ps -e -o pid,comm output string.
 * Always starts with a header line "  PID COMM\n".
 * Followed by num_procs data lines with random PIDs and names.
 * Optionally omits trailing newline on the last line.
 */
static void generate_ps_output(GeneratedInput *gen)
{
    int i;
    char line[512];
    int total_len;
    int has_trailing_newline;

    gen->num_procs = rand_range(0, MAX_TEST_PROCS);
    has_trailing_newline = rand() % 2;

    /* Generate random PIDs and names */
    for (i = 0; i < gen->num_procs; i++) {
        gen->pids[i] = rand_range(1, 99999);
        rand_proc_name(gen->names[i], MAX_NAME_LEN);
    }

    /* Calculate total buffer size needed */
    total_len = 32; /* header + some slack */
    for (i = 0; i < gen->num_procs; i++)
        total_len += 20 + (int)strlen(gen->names[i]);

    gen->text = malloc((size_t)total_len);
    if (!gen->text)
        return;

    /* Write header */
    strcpy(gen->text, "  PID COMM\n");

    /* Write data lines */
    for (i = 0; i < gen->num_procs; i++) {
        /* Mimic ps formatting: right-aligned PID with leading spaces */
        int leading_spaces = rand_range(1, 4);
        char spaces[8] = "";
        int s;
        for (s = 0; s < leading_spaces; s++)
            spaces[s] = ' ';
        spaces[leading_spaces] = '\0';

        if (i == gen->num_procs - 1 && !has_trailing_newline)
            snprintf(line, sizeof(line), "%s%d %s", spaces, gen->pids[i], gen->names[i]);
        else
            snprintf(line, sizeof(line), "%s%d %s\n", spaces, gen->pids[i], gen->names[i]);

        strcat(gen->text, line);
    }
}

static void free_generated(GeneratedInput *gen)
{
    if (gen->text) {
        free(gen->text);
        gen->text = NULL;
    }
}

/* ── Property 3a: Count equals number of data lines ─────────────────── */

/**
 * For any randomly generated ps output with N data lines,
 * process_list_parse must return count == N.
 */
static void test_count_equals_data_lines(void)
{
    int iter;
    int num_iterations = 500;

    printf("[Property 3a] count == number of data lines\n");

    for (iter = 0; iter < num_iterations; iter++) {
        GeneratedInput gen;
        ProcessList list;
        int rc;

        cur_iter = iter;
        memset(&gen, 0, sizeof(gen));

        generate_ps_output(&gen);
        if (!gen.text)
            continue;

        rc = process_list_parse(gen.text, &list);

        CHECK(rc == 0, "parse returned %d, expected 0 (num_procs=%d)",
              rc, gen.num_procs);
        CHECK(list.count == gen.num_procs,
              "count=%d, expected %d", list.count, gen.num_procs);

        process_list_free(&list);
        free_generated(&gen);
    }
}

/* ── Property 3b: PIDs extracted as correct integers ────────────────── */

/**
 * For any randomly generated ps output, each parsed PID must match
 * the PID that was used to generate the corresponding line.
 */
static void test_pids_extracted_correctly(void)
{
    int iter;
    int num_iterations = 500;

    printf("[Property 3b] PIDs extracted as correct integers\n");

    for (iter = 0; iter < num_iterations; iter++) {
        GeneratedInput gen;
        ProcessList list;
        int rc, i;

        cur_iter = iter;
        memset(&gen, 0, sizeof(gen));

        generate_ps_output(&gen);
        if (!gen.text)
            continue;

        rc = process_list_parse(gen.text, &list);

        if (rc == 0 && list.count == gen.num_procs) {
            for (i = 0; i < gen.num_procs; i++) {
                CHECK(list.entries[i].pid == gen.pids[i],
                      "entry[%d].pid=%d, expected %d",
                      i, list.entries[i].pid, gen.pids[i]);
            }
        }

        process_list_free(&list);
        free_generated(&gen);
    }
}

/* ── Property 3c: Names extracted as correct strings ────────────────── */

/**
 * For any randomly generated ps output, each parsed name must match
 * the name that was used to generate the corresponding line.
 */
static void test_names_extracted_correctly(void)
{
    int iter;
    int num_iterations = 500;

    printf("[Property 3c] Names extracted as correct strings\n");

    for (iter = 0; iter < num_iterations; iter++) {
        GeneratedInput gen;
        ProcessList list;
        int rc, i;

        cur_iter = iter;
        memset(&gen, 0, sizeof(gen));

        generate_ps_output(&gen);
        if (!gen.text)
            continue;

        rc = process_list_parse(gen.text, &list);

        if (rc == 0 && list.count == gen.num_procs) {
            for (i = 0; i < gen.num_procs; i++) {
                CHECK(strcmp(list.entries[i].name, gen.names[i]) == 0,
                      "entry[%d].name='%s', expected '%s'",
                      i, list.entries[i].name, gen.names[i]);
            }
        }

        process_list_free(&list);
        free_generated(&gen);
    }
}

/* ── Property 3d: Empty/header-only input yields count 0 ───────────── */

/**
 * Parsing NULL, empty string, or header-only input must always
 * produce count == 0 and return 0.
 */
static void test_empty_inputs_yield_zero(void)
{
    ProcessList list;
    int rc;

    printf("[Property 3d] Empty/header-only input yields count 0\n");

    cur_iter = 0;

    /* NULL input */
    rc = process_list_parse(NULL, &list);
    CHECK(rc == 0, "NULL: parse returned %d", rc);
    CHECK(list.count == 0, "NULL: count=%d", list.count);
    process_list_free(&list);

    /* Empty string */
    rc = process_list_parse("", &list);
    CHECK(rc == 0, "empty: parse returned %d", rc);
    CHECK(list.count == 0, "empty: count=%d", list.count);
    process_list_free(&list);

    /* Header only */
    rc = process_list_parse("  PID COMM\n", &list);
    CHECK(rc == 0, "header-only: parse returned %d", rc);
    CHECK(list.count == 0, "header-only: count=%d", list.count);
    process_list_free(&list);
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    unsigned int seed = (unsigned int)time(NULL);
    srand(seed);

    printf("=== Property 3: Parseo de lista de procesos ===\n");
    printf("    Validates: Requirements 4.1\n");
    printf("    Seed: %u\n\n", seed);

    test_count_equals_data_lines();
    test_pids_extracted_correctly();
    test_names_extracted_correctly();
    test_empty_inputs_yield_zero();

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
