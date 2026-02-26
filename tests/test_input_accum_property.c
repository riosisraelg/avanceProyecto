/**
 * Property-based test for character accumulation in input buffer (Property 6).
 *
 * **Validates: Requirements 5.2**
 *
 * Property 6: Character accumulation in the input buffer
 *   - For any sequence of printable characters fed one by one,
 *     the buffer contains exactly those characters in the same order,
 *     and cursor_pos == length == number of characters.
 *
 * The test embeds the input_init and input_handle_key logic directly
 * to avoid linking against ncurses. The functions are pure computations
 * (only isprint() from ctype.h is needed).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ── Embedded copies of input module (pure logic only) ──────────────────── */

#define INPUT_BUF_SIZE 256

typedef struct {
    char buffer[INPUT_BUF_SIZE];
    int cursor_pos;
    int length;
} InputLine;

static void input_init(InputLine *line)
{
    if (!line) {
        return;
    }
    memset(line->buffer, 0, INPUT_BUF_SIZE);
    line->cursor_pos = 0;
    line->length = 0;
}

static void input_to_uppercase(char *str)
{
    int i;
    if (!str) {
        return;
    }
    for (i = 0; str[i]; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

/*
 * Embedded KEY_BACKSPACE and KEY_ENTER constants.
 * We define them here to avoid including ncurses headers.
 * The actual values don't matter for this test since we only
 * feed printable characters, but we need them for the function
 * to compile.
 */
#define KEY_BACKSPACE 263
#define KEY_ENTER     343

static int input_handle_key(InputLine *line, int ch)
{
    if (!line) {
        return 0;
    }

    /* Enter */
    if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
        if (line->length == 0) {
            return 0;
        }
        input_to_uppercase(line->buffer);
        return 1;
    }

    /* Backspace */
    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (line->cursor_pos > 0) {
            memmove(&line->buffer[line->cursor_pos - 1],
                    &line->buffer[line->cursor_pos],
                    line->length - line->cursor_pos);
            line->cursor_pos--;
            line->length--;
            line->buffer[line->length] = '\0';
        }
        return 0;
    }

    /* Printable characters */
    if (isprint(ch) && line->length < INPUT_BUF_SIZE - 1) {
        memmove(&line->buffer[line->cursor_pos + 1],
                &line->buffer[line->cursor_pos],
                line->length - line->cursor_pos);
        line->buffer[line->cursor_pos] = (char)ch;
        line->cursor_pos++;
        line->length++;
        line->buffer[line->length] = '\0';
        return 0;
    }

    return 0;
}

/* ── Test helpers ───────────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

static const char *cur_label = "";

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            tests_failed++;                                         \
            fprintf(stderr, "  FAIL [%s]: " fmt "\n",               \
                    cur_label, ##__VA_ARGS__);                      \
        }                                                           \
    } while (0)

/* ── Property 6a: Buffer matches input sequence exactly ─────────────────── */

/**
 * Feed a known string of printable characters one by one.
 * After all characters are fed, the buffer must contain the exact
 * same string, and cursor_pos == length == strlen(input).
 */
static void test_accum_known_strings(void) {
    const char *test_strings[] = {
        "hello",
        "Hello World 123",
        "abc",
        "!@#$%^&*()",
        "a",
        "Test 5002",
        "remote@127.0.0.1:5002> ",
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
        "0123456789",
        " spaces  between ",
        "MiXeD cAsE tExT",
        "~`[]{}|;':\",./<>?",
    };
    int num_strings = sizeof(test_strings) / sizeof(test_strings[0]);
    int i, j, len;
    InputLine line;

    printf("[Property 6a] Buffer matches input sequence exactly\n");

    for (i = 0; i < num_strings; i++) {
        cur_label = test_strings[i];
        len = (int)strlen(test_strings[i]);

        input_init(&line);

        /* Feed each character one by one */
        for (j = 0; j < len; j++) {
            input_handle_key(&line, (unsigned char)test_strings[i][j]);
        }

        CHECK(line.length == len,
              "length=%d, expected=%d", line.length, len);

        CHECK(line.cursor_pos == len,
              "cursor_pos=%d, expected=%d", line.cursor_pos, len);

        CHECK(strcmp(line.buffer, test_strings[i]) == 0,
              "buffer=\"%s\", expected=\"%s\"", line.buffer, test_strings[i]);
    }
}

/* ── Property 6b: Cursor position equals length after each character ────── */

/**
 * After feeding each character, cursor_pos must equal length,
 * and both must increment by 1.
 */
static void test_accum_cursor_tracks_length(void) {
    const char *input = "property test 123!";
    int len = (int)strlen(input);
    int j;
    InputLine line;

    printf("[Property 6b] Cursor position equals length after each char\n");

    cur_label = "incremental";
    input_init(&line);

    for (j = 0; j < len; j++) {
        input_handle_key(&line, (unsigned char)input[j]);

        CHECK(line.cursor_pos == j + 1,
              "after char %d ('%c'): cursor_pos=%d, expected=%d",
              j, input[j], line.cursor_pos, j + 1);

        CHECK(line.length == j + 1,
              "after char %d ('%c'): length=%d, expected=%d",
              j, input[j], line.length, j + 1);
    }
}

/* ── Property 6c: Random printable sequences ────────────────────────────── */

/**
 * Generate random sequences of printable ASCII characters (32–126)
 * of varying lengths. Verify buffer content, cursor, and length.
 */
static void test_accum_random_sequences(void) {
    int num_trials = 200;
    int trial, seq_len, j;
    InputLine line;
    char expected[INPUT_BUF_SIZE];
    char label[64];

    printf("[Property 6c] Random printable sequences accumulate correctly\n");

    srand((unsigned int)time(NULL));

    for (trial = 0; trial < num_trials; trial++) {
        /* Random length from 1 to INPUT_BUF_SIZE - 1 */
        seq_len = 1 + (rand() % (INPUT_BUF_SIZE - 1));

        snprintf(label, sizeof(label), "trial %d (len=%d)", trial, seq_len);
        cur_label = label;

        input_init(&line);
        memset(expected, 0, INPUT_BUF_SIZE);

        for (j = 0; j < seq_len; j++) {
            /* Random printable ASCII: 32 (' ') to 126 ('~') */
            char ch = (char)(32 + (rand() % 95));
            expected[j] = ch;
            input_handle_key(&line, (unsigned char)ch);
        }
        expected[seq_len] = '\0';

        CHECK(line.length == seq_len,
              "length=%d, expected=%d", line.length, seq_len);

        CHECK(line.cursor_pos == seq_len,
              "cursor_pos=%d, expected=%d", line.cursor_pos, seq_len);

        CHECK(strcmp(line.buffer, expected) == 0,
              "buffer content mismatch at length %d", seq_len);
    }
}

/* ── Property 6d: Single character accumulation ─────────────────────────── */

/**
 * For every printable ASCII character (32–126), feeding it alone
 * must result in buffer[0] == ch, length == 1, cursor_pos == 1.
 */
static void test_accum_single_chars(void) {
    int ch;
    InputLine line;
    char label[16];

    printf("[Property 6d] Single printable character accumulation\n");

    for (ch = 32; ch <= 126; ch++) {
        snprintf(label, sizeof(label), "char %d ('%c')", ch, ch);
        cur_label = label;

        input_init(&line);
        input_handle_key(&line, ch);

        CHECK(line.length == 1,
              "length=%d, expected=1", line.length);

        CHECK(line.cursor_pos == 1,
              "cursor_pos=%d, expected=1", line.cursor_pos);

        CHECK(line.buffer[0] == (char)ch,
              "buffer[0]='%c' (0x%02x), expected='%c' (0x%02x)",
              line.buffer[0], (unsigned char)line.buffer[0],
              (char)ch, (unsigned char)ch);

        CHECK(line.buffer[1] == '\0',
              "buffer not null-terminated after single char");
    }
}

/* ── Property 6e: handle_key returns 0 for printable characters ─────────── */

/**
 * input_handle_key must return 0 for every printable character
 * (only Enter with non-empty buffer returns 1).
 */
static void test_accum_returns_zero(void) {
    int ch, ret;
    InputLine line;
    char label[16];

    printf("[Property 6e] handle_key returns 0 for printable chars\n");

    for (ch = 32; ch <= 126; ch++) {
        snprintf(label, sizeof(label), "char %d ('%c')", ch, ch);
        cur_label = label;

        input_init(&line);
        ret = input_handle_key(&line, ch);

        CHECK(ret == 0,
              "input_handle_key returned %d, expected 0", ret);
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 6: Character accumulation in input buffer ===\n");
    printf("    Validates: Requirements 5.2\n\n");

    test_accum_known_strings();
    test_accum_cursor_tracks_length();
    test_accum_random_sequences();
    test_accum_single_chars();
    test_accum_returns_zero();

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
