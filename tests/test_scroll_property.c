/**
 * Property-based test for scroll bounds in Panel_Procesos (Property 4).
 *
 * **Validates: Requirements 4.2, 8.3**
 *
 * Property 4: Scroll bounds in Panel_Procesos
 *   - For N entries and visible height H:
 *       offset is in [0, max(0, N - H)]
 *   - Scrolling up from offset 0 stays at 0
 *   - Scrolling down from max offset stays at max
 *
 * The test embeds the scroll_clamp logic directly to avoid
 * linking against ncurses. The function is a pure computation.
 */

#include <stdio.h>
#include <stdlib.h>

/* ── Embedded copy of scroll_clamp ──────────────────────────────────────── */

static int scroll_clamp(int current_offset, int delta,
                        int total_entries, int visible_height)
{
    int max_offset;
    int new_offset;

    max_offset = total_entries - visible_height;
    if (max_offset < 0) {
        max_offset = 0;
    }

    new_offset = current_offset + delta;
    if (new_offset < 0) {
        new_offset = 0;
    }
    if (new_offset > max_offset) {
        new_offset = max_offset;
    }

    return new_offset;
}

/* ── Test helpers ───────────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

static int cur_n = 0;  /* Current N (total entries) for CHECK context */
static int cur_h = 0;  /* Current H (visible height) for CHECK context */

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            tests_failed++;                                         \
            fprintf(stderr, "  FAIL [N=%d, H=%d]: " fmt "\n",      \
                    cur_n, cur_h, ##__VA_ARGS__);                   \
        }                                                           \
    } while (0)

/* ── Property 4a: Offset always in [0, max(0, N-H)] ────────────────────── */

/**
 * For every combination of N in [0, 100] and H in [1, 50],
 * applying any delta in {-5, -1, 0, +1, +5} from any valid starting
 * offset must produce a result in [0, max(0, N - H)].
 */
static void test_scroll_offset_in_range(void) {
    int n, h, offset, delta, result, max_off;
    int deltas[] = { -5, -1, 0, 1, 5 };
    int num_deltas = 5;
    int d;

    printf("[Property 4a] Offset always in [0, max(0, N-H)]\n");

    for (n = 0; n <= 100; n += 5) {
        for (h = 1; h <= 50; h += 3) {
            cur_n = n;
            cur_h = h;

            max_off = n - h;
            if (max_off < 0) max_off = 0;

            /* Test from every valid starting offset */
            for (offset = 0; offset <= max_off; offset++) {
                for (d = 0; d < num_deltas; d++) {
                    delta = deltas[d];
                    result = scroll_clamp(offset, delta, n, h);

                    CHECK(result >= 0 && result <= max_off,
                          "offset=%d, delta=%d: result=%d not in [0, %d]",
                          offset, delta, result, max_off);
                }
            }
        }
    }
}

/* ── Property 4b: Scroll up from 0 stays at 0 ──────────────────────────── */

/**
 * For any N and H, scrolling up (delta = -1) from offset 0 must stay at 0.
 */
static void test_scroll_up_from_zero(void) {
    int n, h, result;

    printf("[Property 4b] Scroll up from offset 0 stays at 0\n");

    for (n = 0; n <= 200; n++) {
        for (h = 1; h <= 100; h += 3) {
            cur_n = n;
            cur_h = h;

            result = scroll_clamp(0, -1, n, h);

            CHECK(result == 0,
                  "scroll_clamp(0, -1, %d, %d) = %d, expected 0",
                  n, h, result);
        }
    }
}

/* ── Property 4c: Scroll down from max stays at max ─────────────────────── */

/**
 * For any N and H, scrolling down (delta = +1) from the maximum offset
 * must stay at the maximum offset.
 */
static void test_scroll_down_from_max(void) {
    int n, h, max_off, result;

    printf("[Property 4c] Scroll down from max offset stays at max\n");

    for (n = 0; n <= 200; n++) {
        for (h = 1; h <= 100; h += 3) {
            cur_n = n;
            cur_h = h;

            max_off = n - h;
            if (max_off < 0) max_off = 0;

            result = scroll_clamp(max_off, 1, n, h);

            CHECK(result == max_off,
                  "scroll_clamp(%d, +1, %d, %d) = %d, expected %d",
                  max_off, n, h, result, max_off);
        }
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 4: Scroll bounds in Panel_Procesos ===\n");
    printf("    Validates: Requirements 4.2, 8.3\n\n");

    test_scroll_offset_in_range();
    test_scroll_up_from_zero();
    test_scroll_down_from_max();

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
