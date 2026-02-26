#ifndef COLORS_H
#define COLORS_H

/* Pares de colores para la TUI */
#define COLOR_PAIR_TEXT      1  /* Gris claro sobre negro */
#define COLOR_PAIR_BORDER    2  /* Gris medio sobre negro */
#define COLOR_PAIR_HEADER    3  /* Blanco sobre gris oscuro */
#define COLOR_PAIR_SELECTED  4  /* Negro sobre gris claro */
#define COLOR_PAIR_ERROR     5  /* Blanco sobre gris oscuro (para errores) */

/* Número total de pares de colores definidos */
#define COLOR_PAIR_COUNT     5

/*
 * Índices ANSI 256-color para la escala de grises (rango 232–255).
 * Estos se usan como referencia para los colores custom definidos
 * con init_color() en colors_init().
 */
#define GRAY_BLACK       232  /* Negro */
#define GRAY_DARK        238  /* Gris oscuro */
#define GRAY_MEDIUM      245  /* Gris medio */
#define GRAY_LIGHT       250  /* Gris claro */
#define GRAY_WHITE       255  /* Blanco */

/**
 * Inicializa la paleta de grises.
 * Llama a start_color(), define colores custom con init_color()
 * y crea los pares de colores con init_pair().
 *
 * Retorna 0 si OK, -1 si la terminal no soporta colores (modo monocromático).
 */
int colors_init(void);

/**
 * Verifica si la terminal soporta colores.
 * Retorna 1 si soporta colores, 0 si no.
 */
int colors_supported(void);

#endif /* COLORS_H */
