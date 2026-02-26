#include <stdlib.h>
#include <string.h>
#include "panels.h"
#include "colors.h"

/*
 * Altura de la Barra_Estado: 1 fila de contenido + 2 filas de borde = 3.
 */
#define STATUS_HEIGHT 3

/*
 * Altura del Panel_Entrada: 3 filas de contenido + 2 filas de borde = 5.
 * El diseño especifica max(3, 5) = 5.
 */
#define INPUT_HEIGHT 5

/*
 * Calcula las dimensiones de los paneles para un tamaño de terminal dado.
 * Esta función es pura (sin efectos secundarios) para facilitar testing.
 *
 * Layout (de arriba a abajo):
 *   Panel_Procesos: ocupa el espacio restante (≥ 60% de LINES)
 *   Panel_Entrada:  INPUT_HEIGHT filas (mínimo 3 filas de contenido + 2 borde = 5)
 *   Barra_Estado:   STATUS_HEIGHT filas (en la parte inferior)
 *
 * Restricciones (Requisitos 2.2–2.5):
 *   - Panel_Procesos ≥ 60% de LINES
 *   - Panel_Entrada  ≥ 3 filas de contenido (input_h ≥ 5 con bordes)
 *   - Barra_Estado   = 1 fila de contenido  (status_h = 3 con bordes)
 *   - proc_h + input_h + status_h = LINES
 */
void panels_calc_dimensions(int lines, int cols,
                            int *proc_h, int *input_h, int *status_h)
{
    int min_proc;

    (void)cols;  /* El ancho no afecta el cálculo de alturas */

    *status_h = STATUS_HEIGHT;
    *input_h  = INPUT_HEIGHT;
    *proc_h   = lines - *input_h - *status_h;

    /*
     * Garantizar que Panel_Procesos ocupe al menos el 60% de la altura.
     * Si no se cumple, reducir Panel_Entrada (manteniendo mínimo de 5 filas
     * = 3 contenido + 2 borde).
     */
    min_proc = (lines * 60 + 99) / 100;  /* ceil(lines * 0.6) */
    if (*proc_h < min_proc) {
        *proc_h  = min_proc;
        *input_h = lines - *proc_h - *status_h;
        /* Asegurar que Panel_Entrada tenga al menos 5 filas (3 contenido + 2 borde) */
        if (*input_h < 5) {
            *input_h = 5;
            *proc_h  = lines - *input_h - *status_h;
        }
    }

    /* Garantizar que Panel_Procesos tenga al menos 2 filas (mínimo funcional) */
    if (*proc_h < 2) {
        *proc_h = 2;
    }
}

/*
 * Crea una ventana ncurses para un panel con las dimensiones dadas.
 */
static WINDOW *create_panel_window(int height, int width, int y, int x)
{
    WINDOW *win = newwin(height, width, y, x);
    if (win) {
        keypad(win, TRUE);
    }
    return win;
}

/*
 * Rellena los campos de un Panel con las dimensiones y crea su ventana.
 */
static void init_panel(Panel *p, int height, int width, int y, int x)
{
    p->height = height;
    p->width  = width;
    p->y      = y;
    p->x      = x;
    p->win    = create_panel_window(height, width, y, x);
}

TUILayout *panels_create(void)
{
    TUILayout *layout;
    int proc_h, input_h, status_h;
    int w;

    layout = (TUILayout *)calloc(1, sizeof(TUILayout));
    if (!layout) {
        return NULL;
    }

    panels_calc_dimensions(LINES, COLS, &proc_h, &input_h, &status_h);
    w = COLS;

    /*
     * Orden vertical (de arriba a abajo):
     *   y=0:                    Panel_Procesos
     *   y=proc_h:              Panel_Entrada
     *   y=proc_h + input_h:    Barra_Estado
     */
    init_panel(&layout->proc,   proc_h,   w, 0,      0);
    init_panel(&layout->input,  input_h,  w, proc_h,  0);
    init_panel(&layout->status, status_h, w, proc_h + input_h, 0);

    layout->focused = 1;  /* Foco inicial en Panel_Entrada */

    return layout;
}

void panels_resize(TUILayout *layout)
{
    int proc_h, input_h, status_h;
    int w;

    if (!layout) {
        return;
    }

    /* Eliminar ventanas antiguas */
    if (layout->proc.win) {
        delwin(layout->proc.win);
        layout->proc.win = NULL;
    }
    if (layout->input.win) {
        delwin(layout->input.win);
        layout->input.win = NULL;
    }
    if (layout->status.win) {
        delwin(layout->status.win);
        layout->status.win = NULL;
    }

    /* Recalcular con las nuevas dimensiones de la terminal */
    panels_calc_dimensions(LINES, COLS, &proc_h, &input_h, &status_h);
    w = COLS;

    /* Recrear ventanas con las nuevas dimensiones */
    init_panel(&layout->proc,   proc_h,   w, 0,      0);
    init_panel(&layout->input,  input_h,  w, proc_h,  0);
    init_panel(&layout->status, status_h, w, proc_h + input_h, 0);
}

/*
 * Dibuja el borde de un panel con el par de color BORDER
 * y escribe el título centrado con el par de color HEADER.
 */
static void draw_panel_border(Panel *p, const char *title)
{
    int title_len, title_x;

    if (!p->win) {
        return;
    }

    /* Dibujar borde con color BORDER */
    wattron(p->win, COLOR_PAIR(COLOR_PAIR_BORDER));
    box(p->win, 0, 0);
    wattroff(p->win, COLOR_PAIR(COLOR_PAIR_BORDER));

    /* Dibujar título centrado en la fila superior del borde */
    if (title) {
        title_len = (int)strlen(title);
        title_x = (p->width - title_len - 2) / 2;  /* -2 para espacios alrededor */
        if (title_x < 1) {
            title_x = 1;
        }

        wattron(p->win, COLOR_PAIR(COLOR_PAIR_HEADER));
        mvwprintw(p->win, 0, title_x, " %s ", title);
        wattroff(p->win, COLOR_PAIR(COLOR_PAIR_HEADER));
    }
}

void panels_draw_borders(TUILayout *layout)
{
    if (!layout) {
        return;
    }

    draw_panel_border(&layout->proc,   "Procesos");
    draw_panel_border(&layout->input,  "Entrada");
    draw_panel_border(&layout->status, "Estado");

    /* Refrescar todas las ventanas */
    if (layout->proc.win) {
        wrefresh(layout->proc.win);
    }
    if (layout->input.win) {
        wrefresh(layout->input.win);
    }
    if (layout->status.win) {
        wrefresh(layout->status.win);
    }
}

void panels_destroy(TUILayout *layout)
{
    if (!layout) {
        return;
    }

    if (layout->proc.win) {
        delwin(layout->proc.win);
        layout->proc.win = NULL;
    }
    if (layout->input.win) {
        delwin(layout->input.win);
        layout->input.win = NULL;
    }
    if (layout->status.win) {
        delwin(layout->status.win);
        layout->status.win = NULL;
    }

    free(layout);
}

/*
 * Calcula el offset de scroll válido para el Panel_Procesos.
 *
 * El offset se clampea al rango [0, max(0, total_entries - visible_height)].
 * Si total_entries <= visible_height, el máximo es 0 (no hay scroll).
 */
int scroll_clamp(int current_offset, int delta,
                 int total_entries, int visible_height)
{
    int max_offset;
    int new_offset;

    /* Calcular el máximo offset permitido */
    max_offset = total_entries - visible_height;
    if (max_offset < 0) {
        max_offset = 0;
    }

    /* Aplicar el delta y clampear */
    new_offset = current_offset + delta;
    if (new_offset < 0) {
        new_offset = 0;
    }
    if (new_offset > max_offset) {
        new_offset = max_offset;
    }

    return new_offset;
}

