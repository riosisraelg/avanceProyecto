/**
 * Property-based test for the grayscale palette (Property 2).
 *
 * **Validates: Requirements 3.1, 3.2**
 *
 * Property 2: Colors exclusively in grayscale
 *   - All defined color pairs use foreground and background in range [232, 255]
 *   - At least 4 distinct pairs exist
 *
 * This test verifies the *specification* of the palette using the constants
 * from colors.h. It does NOT require ncurses initialization — it checks the
 * logical properties of the palette definition at compile/run time.
 */

#include <stdio.h>
#include <stdlib.h>

/* Include the color constants from the project header */
#include "colors.h"

/* ── Static table of expected color pair definitions ────────────────────── */

typedef struct {
    int pair_id;
    int foreground;   /* ANSI 256-color index */
    int background;   /* ANSI 256-color index */
    const char *name;
} ColorPairDef;

static const ColorPairDef palette[] = {
    { COLOR_PAIR_TEXT,     GRAY_LIGHT,  GRAY_BLACK, "TEXT"     },
    { COLOR_PAIR_BORDER,   GRAY_MEDIUM, GRAY_BLACK, "BORDER"   },
    { COLOR_PAIR_HEADER,   GRAY_WHITE,  GRAY_DARK,  "HEADER"   },
    { COLOR_PAIR_SELECTED, GRAY_BLACK,  GRAY_LIGHT, "SELECTED" },
    { COLOR_PAIR_ERROR,    GRAY_WHITE,  GRAY_DARK,  "ERROR"    },
};

static const int PALETTE_SIZE = sizeof(palette) / sizeof(palette[0]);

/* ── Grayscale range bounds ─────────────────────────────────────────────── */

#define GRAY_RANGE_MIN 232
#define GRAY_RANGE_MAX 255

/* ── Test helpers ───────────────────────────────────────────────────────── */

static int tests_run    = 0;
static int tests_passed = 0;

#define CHECK(cond, fmt, ...)                                       \
    do {                                                            \
        tests_run++;                                                \
        if (cond) {                                                 \
            tests_passed++;                                         \
        } else {                                                    \
            fprintf(stderr, "  FAIL: " fmt "\n", ##__VA_ARGS__);    \
        }                                                           \
    } while (0)

/* ── Property checks ────────────────────────────────────────────────────── */

/**
 * Property 2a: Every foreground and background value is within [232, 255].
 */
static void test_all_colors_in_grayscale_range(void) {
    printf("[Property 2a] All colors in grayscale range [%d, %d]\n",
           GRAY_RANGE_MIN, GRAY_RANGE_MAX);

    for (int i = 0; i < PALETTE_SIZE; i++) {
        const ColorPairDef *p = &palette[i];

        CHECK(p->foreground >= GRAY_RANGE_MIN && p->foreground <= GRAY_RANGE_MAX,
              "Pair %s (id=%d): foreground %d out of range [%d, %d]",
              p->name, p->pair_id, p->foreground, GRAY_RANGE_MIN, GRAY_RANGE_MAX);

        CHECK(p->background >= GRAY_RANGE_MIN && p->background <= GRAY_RANGE_MAX,
              "Pair %s (id=%d): background %d out of range [%d, %d]",
              p->name, p->pair_id, p->background, GRAY_RANGE_MIN, GRAY_RANGE_MAX);
    }
}

/**
 * Property 2b: At least 4 distinct (fg, bg) combinations exist.
 */
static void test_at_least_4_distinct_pairs(void) {
    printf("[Property 2b] At least 4 distinct (fg, bg) pairs\n");

    int distinct = 0;

    for (int i = 0; i < PALETTE_SIZE; i++) {
        int is_duplicate = 0;
        for (int j = 0; j < i; j++) {
            if (palette[i].foreground == palette[j].foreground &&
                palette[i].background == palette[j].background) {
                is_duplicate = 1;
                break;
            }
        }
        if (!is_duplicate) {
            distinct++;
        }
    }

    CHECK(distinct >= 4,
          "Only %d distinct (fg, bg) pairs found, expected >= 4", distinct);

    /* Also verify COLOR_PAIR_COUNT matches the palette size */
    CHECK(COLOR_PAIR_COUNT == PALETTE_SIZE,
          "COLOR_PAIR_COUNT (%d) != palette size (%d)",
          COLOR_PAIR_COUNT, PALETTE_SIZE);
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 2: Colors exclusively in grayscale ===\n");
    printf("    Validates: Requirements 3.1, 3.2\n\n");

    test_all_colors_in_grayscale_range();
    test_at_least_4_distinct_pairs();

    printf("\nResults: %d/%d checks passed\n", tests_passed, tests_run);

    if (tests_passed == tests_run) {
        printf("PASS\n");
        return 0;
    } else {
        printf("FAIL\n");
        return 1;
    }
}
