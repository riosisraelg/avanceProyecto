/**
 * Property-based test for buffer clearing on Enter (Property 7).
 *
 * **Validates: Requirements 5.3**
 *
 * Property 7: Buffer clearing on Enter
 *   - For any non-empty input buffer, pressing Enter returns 1
 *     (command ready), and after calling input_clear the buffer
 *     length is 0 and cursor_pos is 0.
 *   - For an empty buffer, pressing Enter returns 0 (Req 5.5).
 *
 * The test embeds the pure input logic directly to avoid linking
 * against ncurses.
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

static void input_clear(InputLine *line)
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

/* ── Helper: fill buffer with a string ──────────────────────────────────── */

static void fill_buffer(InputLine *line, const char *str)
{
    int i;
    int len = (int)strlen(str);
    input_init(line);
    for (i = 0; i < len; i++) {
        input_handle_key(line, (unsigned char)str[i]);
    }
}

/* ── Property 7a: Enter on non-empty buffer returns 1, then clear resets ── */

static void test_enter_nonempty_then_clear(void) {
    const char *test_strings[] = {
        "list",
        "start nginx",
        "stop 1234",
        "exit",
        "a",
        "Hello World",
        "LIST",
        "remote command test",
        "!@#$%",
        "abc123",
    };
    int num_strings = sizeof(test_strings) / sizeof(test_strings[0]);
    int i, ret;
    InputLine line;

    printf("[Property 7a] Enter on non-empty buffer returns 1, clear resets\n");

    for (i = 0; i < num_strings; i++) {
        cur_label = test_strings[i];

        fill_buffer(&line, test_strings[i]);

        /* Verify buffer is non-empty before Enter */
        CHECK(line.length > 0,
              "buffer should be non-empty before Enter, length=%d",
              line.length);

        /* Press Enter */
        ret = input_handle_key(&line, '\n');

        CHECK(ret == 1,
              "input_handle_key should return 1 for Enter with non-empty "
              "buffer, got %d", ret);

        /* Caller clears the buffer (simulating tui_run behavior) */
        input_clear(&line);

        CHECK(line.length == 0,
              "after input_clear: length=%d, expected 0", line.length);

        CHECK(line.cursor_pos == 0,
              "after input_clear: cursor_pos=%d, expected 0",
              line.cursor_pos);

        CHECK(line.buffer[0] == '\0',
              "after input_clear: buffer not empty, buffer[0]='%c'",
              line.buffer[0]);
    }
}

/* ── Property 7b: Enter on empty buffer returns 0 (Req 5.5) ────────────── */

static void test_enter_empty_buffer(void) {
    InputLine line;
    int ret;

    printf("[Property 7b] Enter on empty buffer returns 0\n");

    cur_label = "empty-newline";
    input_init(&line);
    ret = input_handle_key(&line, '\n');
    CHECK(ret == 0,
          "Enter ('\\n') on empty buffer should return 0, got %d", ret);
    CHECK(line.length == 0,
          "length should remain 0, got %d", line.length);
    CHECK(line.cursor_pos == 0,
          "cursor_pos should remain 0, got %d", line.cursor_pos);

    cur_label = "empty-cr";
    input_init(&line);
    ret = input_handle_key(&line, '\r');
    CHECK(ret == 0,
          "Enter ('\\r') on empty buffer should return 0, got %d", ret);

    cur_label = "empty-KEY_ENTER";
    input_init(&line);
    ret = input_handle_key(&line, KEY_ENTER);
    CHECK(ret == 0,
          "Enter (KEY_ENTER) on empty buffer should return 0, got %d", ret);
}

/* ── Property 7c: Random non-empty buffers + Enter + clear ──────────────── */

static void test_enter_random_then_clear(void) {
    int num_trials = 200;
    int trial, seq_len, j, ret;
    InputLine line;
    char label[64];

    printf("[Property 7c] Random non-empty buffers: Enter returns 1, "
           "clear resets to 0\n");

    srand((unsigned int)time(NULL));

    for (trial = 0; trial < num_trials; trial++) {
        /* Random length from 1 to INPUT_BUF_SIZE - 1 */
        seq_len = 1 + (rand() % (INPUT_BUF_SIZE - 1));

        snprintf(label, sizeof(label), "trial %d (len=%d)", trial, seq_len);
        cur_label = label;

        input_init(&line);

        /* Fill with random printable characters */
        for (j = 0; j < seq_len; j++) {
            char ch = (char)(32 + (rand() % 95));
            input_handle_key(&line, (unsigned char)ch);
        }

        /* Verify non-empty */
        CHECK(line.length == seq_len,
              "pre-Enter length=%d, expected=%d", line.length, seq_len);

        /* Press Enter */
        ret = input_handle_key(&line, '\n');

        CHECK(ret == 1,
              "input_handle_key should return 1, got %d", ret);

        /* Clear (as tui_run would do) */
        input_clear(&line);

        CHECK(line.length == 0,
              "after clear: length=%d, expected 0", line.length);

        CHECK(line.cursor_pos == 0,
              "after clear: cursor_pos=%d, expected 0", line.cursor_pos);
    }
}

/* ── Property 7d: All Enter key variants work the same ──────────────────── */

static void test_enter_all_variants(void) {
    int enter_keys[] = { '\n', '\r', KEY_ENTER };
    const char *enter_names[] = { "\\n", "\\r", "KEY_ENTER" };
    int num_variants = 3;
    int i, ret;
    InputLine line;

    printf("[Property 7d] All Enter key variants return 1 for non-empty "
           "buffer\n");

    for (i = 0; i < num_variants; i++) {
        cur_label = enter_names[i];

        fill_buffer(&line, "test command");

        ret = input_handle_key(&line, enter_keys[i]);

        CHECK(ret == 1,
              "Enter variant %s should return 1, got %d",
              enter_names[i], ret);

        input_clear(&line);

        CHECK(line.length == 0,
              "after clear with %s: length=%d", enter_names[i], line.length);

        CHECK(line.cursor_pos == 0,
              "after clear with %s: cursor_pos=%d",
              enter_names[i], line.cursor_pos);
    }
}

/* ── Property 7e: Repeated Enter+clear cycles ──────────────────────────── */

static void test_enter_clear_repeated_cycles(void) {
    int cycle, ret;
    InputLine line;
    char label[32];

    printf("[Property 7e] Repeated fill-Enter-clear cycles\n");

    input_init(&line);

    for (cycle = 0; cycle < 50; cycle++) {
        snprintf(label, sizeof(label), "cycle %d", cycle);
        cur_label = label;

        /* Fill with some content */
        fill_buffer(&line, "cmd");

        ret = input_handle_key(&line, '\n');
        CHECK(ret == 1,
              "Enter should return 1 on cycle %d, got %d", cycle, ret);

        input_clear(&line);

        CHECK(line.length == 0,
              "cycle %d: length=%d after clear", cycle, line.length);
        CHECK(line.cursor_pos == 0,
              "cycle %d: cursor_pos=%d after clear", cycle, line.cursor_pos);
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 7: Buffer clearing on Enter ===\n");
    printf("    Validates: Requirements 5.3\n\n");

    test_enter_nonempty_then_clear();
    test_enter_empty_buffer();
    test_enter_random_then_clear();
    test_enter_all_variants();
    test_enter_clear_repeated_cycles();

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
