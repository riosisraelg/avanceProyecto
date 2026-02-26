#include "curses_compat.h"
#include "colors.h"

/*
 * Índices de colores custom para ncurses.
 * Se usan índices a partir de 16 para no sobreescribir los colores por defecto (0–15).
 * Cada uno mapea a un tono de gris en el rango ANSI 232–255.
 */
#define CUSTOM_BLACK    16  /* Mapea a GRAY_BLACK  (232) */
#define CUSTOM_DARK     17  /* Mapea a GRAY_DARK   (238) */
#define CUSTOM_MEDIUM   18  /* Mapea a GRAY_MEDIUM (245) */
#define CUSTOM_LIGHT    19  /* Mapea a GRAY_LIGHT  (250) */
#define CUSTOM_WHITE    20  /* Mapea a GRAY_WHITE  (255) */

/*
 * Convierte un índice ANSI de escala de grises (232–255) a un valor
 * RGB en el rango 0–1000 que espera ncurses init_color().
 *
 * La escala de grises ANSI 256 define 24 tonos:
 *   índice 232 = rgb(8,8,8)     → más oscuro
 *   índice 255 = rgb(238,238,238) → más claro
 *
 * Fórmula: valor_8bit = 8 + (indice - 232) * 10
 *          valor_1000 = valor_8bit * 1000 / 255
 */
static int ansi_gray_to_ncurses(int ansi_index)
{
    int val_8bit = 8 + (ansi_index - 232) * 10;
    return val_8bit * 1000 / 255;
}

/*
 * Inicializa los colores custom usando init_color() con valores
 * derivados de la escala de grises ANSI.
 */
static void init_custom_colors(void)
{
    int v;

    v = ansi_gray_to_ncurses(GRAY_BLACK);
    init_color(CUSTOM_BLACK, v, v, v);

    v = ansi_gray_to_ncurses(GRAY_DARK);
    init_color(CUSTOM_DARK, v, v, v);

    v = ansi_gray_to_ncurses(GRAY_MEDIUM);
    init_color(CUSTOM_MEDIUM, v, v, v);

    v = ansi_gray_to_ncurses(GRAY_LIGHT);
    init_color(CUSTOM_LIGHT, v, v, v);

    v = ansi_gray_to_ncurses(GRAY_WHITE);
    init_color(CUSTOM_WHITE, v, v, v);
}

/*
 * Crea los pares de colores usando los colores custom definidos.
 */
static void init_custom_pairs(void)
{
    /* TEXT:     Gris claro (250) sobre Negro (232) */
    init_pair(COLOR_PAIR_TEXT, CUSTOM_LIGHT, CUSTOM_BLACK);

    /* BORDER:   Gris medio (245) sobre Negro (232) */
    init_pair(COLOR_PAIR_BORDER, CUSTOM_MEDIUM, CUSTOM_BLACK);

    /* HEADER:   Blanco (255) sobre Gris oscuro (238) */
    init_pair(COLOR_PAIR_HEADER, CUSTOM_WHITE, CUSTOM_DARK);

    /* SELECTED: Negro (232) sobre Gris claro (250) */
    init_pair(COLOR_PAIR_SELECTED, CUSTOM_BLACK, CUSTOM_LIGHT);

    /* ERROR:    Blanco (255) sobre Gris oscuro (238) */
    init_pair(COLOR_PAIR_ERROR, CUSTOM_WHITE, CUSTOM_DARK);
}

/*
 * Fallback: usa los colores estándar más cercanos cuando
 * can_change_color() retorna FALSE.
 */
static void init_fallback_pairs(void)
{
    /* TEXT:     Blanco sobre Negro */
    init_pair(COLOR_PAIR_TEXT, COLOR_WHITE, COLOR_BLACK);

    /* BORDER:   Blanco sobre Negro */
    init_pair(COLOR_PAIR_BORDER, COLOR_WHITE, COLOR_BLACK);

    /* HEADER:   Blanco sobre Negro (con A_REVERSE como alternativa visual) */
    init_pair(COLOR_PAIR_HEADER, COLOR_WHITE, COLOR_BLACK);

    /* SELECTED: Negro sobre Blanco */
    init_pair(COLOR_PAIR_SELECTED, COLOR_BLACK, COLOR_WHITE);

    /* ERROR:    Blanco sobre Negro */
    init_pair(COLOR_PAIR_ERROR, COLOR_WHITE, COLOR_BLACK);
}

int colors_init(void)
{
    if (!has_colors()) {
        return -1;  /* Modo monocromático */
    }

    start_color();

    if (can_change_color()) {
        init_custom_colors();
        init_custom_pairs();
    } else {
        init_fallback_pairs();
    }

    return 0;
}

int colors_supported(void)
{
    return has_colors() ? 1 : 0;
}
