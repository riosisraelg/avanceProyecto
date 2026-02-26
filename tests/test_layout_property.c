/**
 * Property-based test for layout constraints (Property 1).
 *
 * **Validates: Requirements 2.2, 2.3, 2.4, 2.5**
 *
 * Property 1: Layout constraints for any terminal size
 *   - For LINES >= 10 and COLS >= 40:
 *       Panel_Procesos >= 60% height
 *       Panel_Entrada  >= 3 content rows (input_h >= 5 with borders)
 *       Barra_Estado   = 1 content row   (status_h = 3 with borders)
 *       sum of heights = LINES
 *   - Recalculating with new dimensions produces the same result
 *     as calculating from scratch (idempotency).
 *
 * The test embeds the panels_calc_dimensions logic directly to avoid
 * linking against ncurses. The function is a pure computation that
 * only depends on its parameters.
 */

#include <stdio.h>
#include <stdlib.h>

/* ── Embedded copy of panels_calc_dimensions ────────────────────────────── */

#define STATUS_HEIGHT 3
#define INPUT_HEIGHT  5

static void panels_calc_dimensions(int lines, int cols,
                                   int *proc_h, int *input_h, int *status_h)
{
    int min_proc;

    (void)cols;

    *status_h = STATUS_HEIGHT;
    *input_h  = INPUT_HEIGHT;
    *proc_h   = lines - *input_h - *status_h;

    /*
     * Garantizar que Panel_Procesos ocupe al menos el 60% de la altura.
     * Si no se cumple, reducir Panel_Entrada (manteniendo mínimo de 5 filas).
     */
    min_proc = (lines * 60 + 99) / 100;  /* ceil(lines * 0.6) */
    if (*proc_h < min_proc) {
        *proc_h  = min_proc;
        *input_h = lines - *proc_h - *status_h;
        if (*input_h < 5) {
            *input_h = 5;
            *proc_h  = lines - *input_h - *status_h;
        }
    }

    if (*proc_h < 2) {
        *proc_h = 2;
    }
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
            fprintf(stderr, "  FAIL [LINES=%d, COLS=%d]: " fmt "\n", \
                    cur_lines, cur_cols, ##__VA_ARGS__);            \
        }                                                           \
    } while (0)

/* Current dimensions being tested (used by CHECK macro for context) */
static int cur_lines = 0;
static int cur_cols  = 0;

/* ── Property 1a: Layout constraints hold for all valid sizes ───────────── */

/**
 * For every (lines, cols) with lines in [10, 200] and cols in [40, 300]:
 *   - proc_h + input_h + status_h == lines   (always)
 *   - input_h >= 5                            (always: 3 content + 2 border)
 *   - status_h == 3                           (always: 1 content + 2 border)
 *   - proc_h * 100 >= lines * 60              (when lines >= 20, all constraints
 *                                              can be satisfied simultaneously)
 *
 * For lines < 20, the 60% constraint may conflict with the minimum
 * input_h=5 and status_h=3 (since 0.6*lines + 5 + 3 > lines when lines < 20).
 * In that case, the sum constraint and minimum heights take priority.
 */
static void test_layout_constraints(void) {
    int lines, cols;
    int proc_h, input_h, status_h;
    int min_proc_60;

    printf("[Property 1a] Layout constraints for LINES=[10..200], COLS=[40..300]\n");

    for (lines = 10; lines <= 200; lines++) {
        for (cols = 40; cols <= 300; cols += 20) {
            cur_lines = lines;
            cur_cols  = cols;

            panels_calc_dimensions(lines, cols, &proc_h, &input_h, &status_h);

            /* Sum of heights must equal LINES */
            CHECK(proc_h + input_h + status_h == lines,
                  "Sum mismatch: proc_h(%d) + input_h(%d) + status_h(%d) = %d, expected %d",
                  proc_h, input_h, status_h,
                  proc_h + input_h + status_h, lines);

            /* Panel_Entrada >= 5 rows (3 content + 2 border) */
            CHECK(input_h >= 5,
                  "input_h(%d) < 5 (minimum 3 content rows + 2 border)",
                  input_h);

            /* Barra_Estado == 3 rows (1 content + 2 border) */
            CHECK(status_h == 3,
                  "status_h(%d) != 3 (expected 1 content row + 2 border)",
                  status_h);

            /*
             * Panel_Procesos >= 60% of total height.
             * This is only achievable when lines >= 20 (since 0.6*20 + 5 + 3 = 20).
             * For smaller terminals, verify proc_h is maximized given the constraints.
             */
            min_proc_60 = (lines * 60 + 99) / 100;  /* ceil(lines * 0.6) */
            if (lines >= 20) {
                CHECK(proc_h >= min_proc_60,
                      "proc_h(%d) < 60%% of lines(%d): need >= %d",
                      proc_h, lines, min_proc_60);
            } else {
                /* For small terminals, proc_h should be lines - 5 - 3 = lines - 8 */
                CHECK(proc_h == lines - 5 - 3,
                      "proc_h(%d) != expected %d for small terminal (lines=%d)",
                      proc_h, lines - 5 - 3, lines);
            }
        }
    }
}

/* ── Property 1b: Resize equivalence — recalculating with new dimensions ── */

/**
 * After calculating layout for dimensions (linesA, colsA), recalculating
 * with (linesB, colsB) must produce the same result as calculating from
 * scratch with (linesB, colsB). This verifies that the function is pure
 * and has no hidden state — essential for correct KEY_RESIZE handling.
 */
static void test_layout_resize_equivalence(void) {
    int linesA, colsA, linesB, colsB;
    int pa, ia, sa;          /* first calc (dimensions A) — discarded */
    int p_resize, i_resize, s_resize;   /* recalc with dimensions B */
    int p_fresh, i_fresh, s_fresh;      /* fresh calc with dimensions B */

    printf("[Property 1b] Resize equivalence: recalc == fresh calc\n");

    /* Sample pairs of (A, B) dimensions */
    for (linesA = 10; linesA <= 100; linesA += 15) {
        for (colsA = 40; colsA <= 200; colsA += 40) {
            for (linesB = 10; linesB <= 100; linesB += 15) {
                for (colsB = 40; colsB <= 200; colsB += 40) {
                    cur_lines = linesB;
                    cur_cols  = colsB;

                    /* Simulate: first layout with A, then resize to B */
                    panels_calc_dimensions(linesA, colsA, &pa, &ia, &sa);
                    panels_calc_dimensions(linesB, colsB,
                                           &p_resize, &i_resize, &s_resize);

                    /* Fresh calculation with B */
                    panels_calc_dimensions(linesB, colsB,
                                           &p_fresh, &i_fresh, &s_fresh);

                    CHECK(p_resize == p_fresh &&
                          i_resize == i_fresh &&
                          s_resize == s_fresh,
                          "Resize from (%d,%d)->(%d,%d): "
                          "resize=(%d,%d,%d) fresh=(%d,%d,%d)",
                          linesA, colsA, linesB, colsB,
                          p_resize, i_resize, s_resize,
                          p_fresh, i_fresh, s_fresh);
                }
            }
        }
    }
}

/* ── Main ───────────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== Property 1: Layout constraints for any terminal size ===\n");
    printf("    Validates: Requirements 2.2, 2.3, 2.4, 2.5\n\n");

    test_layout_constraints();
    test_layout_resize_equivalence();

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
