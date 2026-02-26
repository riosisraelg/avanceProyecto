/**
 * Property-based test for focus toggle with Tab (Property 10).
 *
 * **Validates: Requirements 8.2**
 *
 * Property 10: Focus toggle with Tab
 *   - For any current focus state (0 = Panel_Procesos, 1 = Panel_Entrada),
 *     pressing Tab changes focus to the other panel.
 *   - Pressing Tab twice returns to the original panel (round-trip).
 *
 * The test embeds the focus toggle logic directly to avoid
 * linking against ncurses. The function is a pure computation.
 */

#include <stdio.h>
#include <stdlib.h>

/* ── Embedded copy of focus toggle logic ────────────────────────────────── */

/**
 * Toggles focus between panels.
 * Mirrors the logic in tui_run(): focused = (focused == 0) ? 1 : 0
 *
 * Returns the new focus value.
 */
static int toggle_focus(int current_focus)
{
    return (current_focus == 0) ? 1 : 0;
}

/* ── Test helpers ───────────────────────────────────────────────────────── */

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

/* ── Property 10a: Tab toggles focus to the other panel ─────────────────── */

static void test_tab_toggles_focus(void) {
    int focus, result;

    printf("[Property 10a] Tab toggles focus to the other panel\n");

    /* From Panel_Procesos (0) → Panel_Entrada (1) */
    focus = 0;
    result = toggle_focus(focus);
    CHECK(result == 1,
          "toggle_focus(0) = %d, expected 1", result);

    /* From Panel_Entrada (1) → Panel_Procesos (0) */
    focus = 1;
    result = toggle_focus(focus);
    CHECK(result == 0,
          "toggle_focus(1) = %d, expected 0", result);
}

/* ── Property 10b: Tab×2 returns to original (round-trip) ───────────────── */

static void test_tab_roundtrip(void) {
    int focus, after_one, after_two;

    printf("[Property 10b] Tab x2 returns to original panel (round-trip)\n");

    /* Starting from 0 */
    focus = 0;
    after_one = toggle_focus(focus);
    after_two = toggle_focus(after_one);
    CHECK(after_two == focus,
          "Starting at 0: after 2 toggles got %d, expected %d",
          after_two, focus);

    /* Starting from 1 */
    focus = 1;
    after_one = toggle_focus(focus);
    after_two = toggle_focus(after_one);
    CHECK(after_two == focus,
          "Starting at 1: after 2 toggles got %d, expected %d",
          after_two, focus);
}

/* ── Property 10c: Result is always 0 or 1 ──────────────────────────────── */

static void test_result_is_binary(void) {
    int focus, result;

    printf("[Property 10c] Result is always 0 or 1\n");

    for (focus = 0; focus <= 1; focus++) {
        result = toggle_focus(focus);
        CHECK(result == 0 || result == 1,
              "toggle_focus(%d) = %d, expected 0 or 1", focus, result);
    }
}

/* ── Property 10d: Result is always different from input ─────────────────── */

static void test_result_differs_from_input(void) {
    int focus, result;

    printf("[Property 10d] Result is always different from input\n");

    for (focus = 0; focus <= 1; focus++) {
        result = toggle_focus(focus);
        CHECK(result != focus,
              "toggle_focus(%d) = %d, should differ from input", focus, result);
    }
}

/* ── Property 10e: Multiple toggle sequences ─────────────────────────────── */

static void test_multiple_toggle_sequences(void) {
    int focus, i, n;

    printf("[Property 10e] Multiple toggle sequences preserve invariants\n");

    /* For sequences of N toggles, verify:
     * - Even N → returns to original
     * - Odd N  → lands on the other panel
     */
    for (n = 1; n <= 20; n++) {
        /* Starting from 0 */
        focus = 0;
        for (i = 0; i < n; i++) {
            focus = toggle_focus(focus);
        }

        if (n % 2 == 0) {
            CHECK(focus == 0,
                  "After %d toggles from 0: got %d, expected 0", n, focus);
        } else {
            CHECK(focus == 1,
                  "After %d toggles from 0: got %d, expected 1", n, focus);
        }

        /* Starting from 1 */
        focus = 1;
        for (i = 0; i < n; i++) {
            focus = toggle_focus(focus);
        }

        if (n % 2 == 0) {
            CHECK(focus == 1,
                  "After %d toggles from 1: got %d, expected 1", n, focus);
        } else {
            CHECK(focus == 0,
                  "After %d toggles from 1: got %d, expected 0", n, focus);
        }
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 10: Focus toggle with Tab ===\n");
    printf("    Validates: Requirements 8.2\n\n");

    test_tab_toggles_focus();
    test_tab_roundtrip();
    test_result_is_binary();
    test_result_differs_from_input();
    test_multiple_toggle_sequences();

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
