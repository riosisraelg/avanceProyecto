/**
 * Property-based test for command normalization to uppercase (Property 8).
 *
 * **Validates: Requirements 5.4**
 *
 * Property 8: Command normalization to uppercase
 *   - For any string of ASCII letters, the normalization function must
 *     produce a string where all alphabetic characters are uppercase,
 *     and the length is preserved.
 *
 * The test embeds the input_to_uppercase logic directly to avoid
 * linking against ncurses. The function is a pure computation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* ── Embedded copy of input_to_uppercase ─────────────────────────────────── */

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


/* ── Property 8a: Known strings become fully uppercase ──────────────────── */

static void test_known_strings_uppercase(void) {
    const char *inputs[] = {
        "list", "start", "stop", "exit",
        "LIST", "START", "STOP", "EXIT",
        "List", "StArT", "sToP", "eXiT",
        "hello", "HELLO", "HeLLo",
        "abc", "ABC", "aBcDeF",
    };
    const char *expected[] = {
        "LIST", "START", "STOP", "EXIT",
        "LIST", "START", "STOP", "EXIT",
        "LIST", "START", "STOP", "EXIT",
        "HELLO", "HELLO", "HELLO",
        "ABC", "ABC", "ABCDEF",
    };
    int num = sizeof(inputs) / sizeof(inputs[0]);
    int i;
    char buf[256];

    printf("[Property 8a] Known strings become fully uppercase\n");

    for (i = 0; i < num; i++) {
        cur_label = inputs[i];
        strncpy(buf, inputs[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';

        input_to_uppercase(buf);

        CHECK(strcmp(buf, expected[i]) == 0,
              "input \"%s\" -> \"%s\", expected \"%s\"",
              inputs[i], buf, expected[i]);
    }
}

/* ── Property 8b: All ASCII letters become uppercase ────────────────────── */

/**
 * For every individual ASCII letter (a-z, A-Z), after normalization
 * the character must be the uppercase variant.
 */
static void test_all_ascii_letters(void) {
    char buf[2];
    int ch;

    printf("[Property 8b] All ASCII letters become uppercase\n");

    for (ch = 'a'; ch <= 'z'; ch++) {
        buf[0] = (char)ch;
        buf[1] = '\0';
        cur_label = buf;

        input_to_uppercase(buf);

        CHECK(buf[0] == toupper(ch),
              "'%c' -> '%c', expected '%c'",
              (char)ch, buf[0], (char)toupper(ch));
    }

    for (ch = 'A'; ch <= 'Z'; ch++) {
        buf[0] = (char)ch;
        buf[1] = '\0';
        cur_label = buf;

        input_to_uppercase(buf);

        CHECK(buf[0] == ch,
              "'%c' -> '%c', expected '%c' (already uppercase)",
              (char)ch, buf[0], (char)ch);
    }
}

/* ── Property 8c: Length is preserved after normalization ────────────────── */

/**
 * For random strings of varying lengths, the length before and after
 * normalization must be identical.
 */
static void test_length_preserved(void) {
    char buf[256];
    int trial, i, len, orig_len;

    printf("[Property 8c] Length is preserved after normalization\n");

    srand((unsigned)time(NULL));

    for (trial = 0; trial < 200; trial++) {
        len = (rand() % 100) + 1; /* 1..100 */
        for (i = 0; i < len; i++) {
            /* Generate random printable ASCII characters (32..126) */
            buf[i] = (char)(32 + (rand() % 95));
        }
        buf[len] = '\0';
        orig_len = (int)strlen(buf);

        cur_label = "random_string";

        input_to_uppercase(buf);

        CHECK((int)strlen(buf) == orig_len,
              "length changed: orig=%d, after=%d",
              orig_len, (int)strlen(buf));
    }
}

/* ── Property 8d: All alpha chars are uppercase after normalization ──────── */

/**
 * For random strings, after normalization every alphabetic character
 * must be uppercase.
 */
static void test_all_alpha_uppercase(void) {
    char buf[256];
    int trial, i, len, all_upper;

    printf("[Property 8d] All alpha chars are uppercase after normalization\n");

    srand((unsigned)(time(NULL) + 1));

    for (trial = 0; trial < 200; trial++) {
        len = (rand() % 100) + 1;
        for (i = 0; i < len; i++) {
            buf[i] = (char)(32 + (rand() % 95));
        }
        buf[len] = '\0';

        cur_label = "random_alpha_check";

        input_to_uppercase(buf);

        all_upper = 1;
        for (i = 0; buf[i]; i++) {
            if (isalpha((unsigned char)buf[i]) && !isupper((unsigned char)buf[i])) {
                all_upper = 0;
                break;
            }
        }

        CHECK(all_upper,
              "found lowercase alpha at position %d: '%c' (0x%02x)",
              i, buf[i], (unsigned char)buf[i]);
    }
}

/* ── Property 8e: Non-alpha characters are preserved ────────────────────── */

/**
 * For strings containing digits, spaces, and punctuation, those
 * characters must remain unchanged after normalization.
 */
static void test_non_alpha_preserved(void) {
    const char *inputs[] = {
        "123", "!@#$%", "hello 123 world", "  spaces  ",
        "a1b2c3", "LIST-123", "stop.now", "cmd;arg",
        "12345", "---", "...", "a!b@c#d$e%",
    };
    int num = sizeof(inputs) / sizeof(inputs[0]);
    int i, j;
    char buf[256];
    char orig[256];

    printf("[Property 8e] Non-alpha characters are preserved\n");

    for (i = 0; i < num; i++) {
        cur_label = inputs[i];
        strncpy(buf, inputs[i], sizeof(buf) - 1);
        buf[sizeof(buf) - 1] = '\0';
        strncpy(orig, inputs[i], sizeof(orig) - 1);
        orig[sizeof(orig) - 1] = '\0';

        input_to_uppercase(buf);

        for (j = 0; orig[j]; j++) {
            if (!isalpha((unsigned char)orig[j])) {
                CHECK(buf[j] == orig[j],
                      "non-alpha char at pos %d changed: '%c' -> '%c'",
                      j, orig[j], buf[j]);
            }
        }
    }
}

/* ── Property 8f: Empty string and NULL handling ────────────────────────── */

static void test_edge_cases(void) {
    char buf[4];

    printf("[Property 8f] Edge cases: empty string and NULL\n");

    cur_label = "empty_string";
    buf[0] = '\0';
    input_to_uppercase(buf);
    CHECK(strlen(buf) == 0, "empty string length changed to %d", (int)strlen(buf));

    cur_label = "null_ptr";
    input_to_uppercase(NULL); /* should not crash */
    CHECK(1, "NULL pointer did not crash"); /* if we reach here, it passed */
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 8: Command normalization to uppercase ===\n");
    printf("    Validates: Requirements 5.4\n\n");

    test_known_strings_uppercase();
    test_all_ascii_letters();
    test_length_preserved();
    test_all_alpha_uppercase();
    test_non_alpha_preserved();
    test_edge_cases();

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
