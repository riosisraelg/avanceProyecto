#ifndef PANELS_H
#define PANELS_H

#include "curses_compat.h"

typedef struct {
    WINDOW *win;          /* Ventana ncurses */
    int y, x;             /* Posición */
    int height, width;    /* Dimensiones */
} Panel;

typedef struct {
    Panel proc;           /* Panel_Procesos */
    Panel input;          /* Panel_Entrada */
    Panel status;         /* Barra_Estado */
    int focused;          /* 0 = Panel_Procesos, 1 = Panel_Entrada */
} TUILayout;

/* Crea el layout inicial basado en las dimensiones de la terminal. */
TUILayout *panels_create(void);

/* Recalcula dimensiones y redibuja tras un cambio de tamaño. */
void panels_resize(TUILayout *layout);

/* Dibuja los bordes y títulos de todos los paneles. */
void panels_draw_borders(TUILayout *layout);

/* Libera la memoria de todas las ventanas. */
void panels_destroy(TUILayout *layout);

/*
 * Calcula las dimensiones de los paneles para un tamaño de terminal dado.
 * Función pura expuesta para testing (no crea ventanas).
 * proc_h, input_h, status_h reciben las alturas calculadas.
 */
void panels_calc_dimensions(int lines, int cols,
                            int *proc_h, int *input_h, int *status_h);

/*
 * Calcula el offset de scroll válido para el Panel_Procesos.
 * Función pura expuesta para testing.
 *
 * Parámetros:
 *   current_offset - offset actual de scroll
 *   delta          - cambio solicitado (+1 scroll abajo, -1 scroll arriba)
 *   total_entries  - número total de entradas (N)
 *   visible_height - filas visibles en el panel (H)
 *
 * Retorna el nuevo offset clampeado en [0, max(0, N - H)].
 */
int scroll_clamp(int current_offset, int delta,
                 int total_entries, int visible_height);

#endif /* PANELS_H */
