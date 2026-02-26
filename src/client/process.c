#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "process.h"
#include "colors.h"

#define INITIAL_CAPACITY 32

/*
 * Parsea la respuesta cruda del servidor (texto de `ps -e -o pid,comm`)
 * en una ProcessList. La primera línea (encabezado "PID COMM") se omite.
 *
 * Formato esperado por línea (después del header):
 *   "  1234 nginx"
 *   "  5678 node"
 *
 * Retorna 0 si OK, -1 en error de memoria.
 */
int process_list_parse(const char *raw_response, ProcessList *list)
{
    const char *line_start;
    const char *p;
    int is_first_line;

    if (!list)
        return -1;

    list->entries  = NULL;
    list->count    = 0;
    list->capacity = 0;

    if (!raw_response || raw_response[0] == '\0')
        return 0;

    /* Asignar capacidad inicial */
    list->entries = malloc(INITIAL_CAPACITY * sizeof(ProcessEntry));
    if (!list->entries)
        return -1;
    list->capacity = INITIAL_CAPACITY;

    is_first_line = 1;
    line_start = raw_response;

    while (*line_start != '\0') {
        /* Encontrar el fin de la línea */
        p = line_start;
        while (*p != '\0' && *p != '\n')
            p++;

        /* Longitud de la línea */
        int line_len = (int)(p - line_start);

        if (is_first_line) {
            /* Omitir la línea de encabezado */
            is_first_line = 0;
        } else if (line_len > 0) {
            /* Parsear PID y nombre */
            const char *s = line_start;

            /* Saltar espacios iniciales */
            while (s < line_start + line_len && isspace((unsigned char)*s))
                s++;

            if (s < line_start + line_len) {
                /* Leer PID */
                int pid = 0;
                int has_digit = 0;
                while (s < line_start + line_len && isdigit((unsigned char)*s)) {
                    pid = pid * 10 + (*s - '0');
                    has_digit = 1;
                    s++;
                }

                if (has_digit) {
                    /* Saltar espacios entre PID y nombre */
                    while (s < line_start + line_len && isspace((unsigned char)*s))
                        s++;

                    /* El resto es el nombre del proceso */
                    int name_len = (int)(line_start + line_len - s);

                    /* Crecer el array si es necesario */
                    if (list->count >= list->capacity) {
                        int new_cap = list->capacity * 2;
                        ProcessEntry *tmp = realloc(list->entries,
                                                    (size_t)new_cap * sizeof(ProcessEntry));
                        if (!tmp)
                            return -1;
                        list->entries  = tmp;
                        list->capacity = new_cap;
                    }

                    ProcessEntry *entry = &list->entries[list->count];
                    entry->pid = pid;

                    if (name_len > 0 && name_len < PROC_NAME_SIZE) {
                        memcpy(entry->name, s, (size_t)name_len);
                        entry->name[name_len] = '\0';
                    } else if (name_len >= PROC_NAME_SIZE) {
                        memcpy(entry->name, s, PROC_NAME_SIZE - 1);
                        entry->name[PROC_NAME_SIZE - 1] = '\0';
                    } else {
                        entry->name[0] = '\0';
                    }

                    list->count++;
                }
            }
        }

        /* Avanzar a la siguiente línea */
        if (*p == '\n')
            line_start = p + 1;
        else
            break;
    }

    return 0;
}

/*
 * Libera la memoria de la lista de procesos.
 */
void process_list_free(ProcessList *list)
{
    if (!list)
        return;

    if (list->entries) {
        free(list->entries);
        list->entries = NULL;
    }
    list->count    = 0;
    list->capacity = 0;
}

/*
 * Renderiza la lista de procesos en el Panel_Procesos con columnas alineadas.
 * Muestra "Sin procesos activos" centrado si la lista está vacía.
 */
void process_list_render(const ProcessList *list, Panel *panel, int scroll_offset)
{
    int inner_h, inner_w;
    int row;
    int i;

    if (!panel || !panel->win)
        return;

    /* Área interior (sin bordes) */
    inner_h = panel->height - 2;
    inner_w = panel->width - 2;

    if (inner_h <= 0 || inner_w <= 0)
        return;

    /* Limpiar el área interior */
    for (row = 1; row <= inner_h; row++) {
        wmove(panel->win, row, 1);
        for (i = 0; i < inner_w; i++)
            waddch(panel->win, ' ');
    }

    if (!list || list->count == 0) {
        /* Mostrar "Sin procesos activos" centrado */
        const char *msg = "Sin procesos activos";
        int msg_len = (int)strlen(msg);
        int cx = (inner_w - msg_len) / 2 + 1;
        int cy = inner_h / 2 + 1;

        if (cx < 1) cx = 1;
        if (cy < 1) cy = 1;

        wattron(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(panel->win, cy, cx, "%s", msg);
        wattroff(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
    } else {
        /* Dibujar encabezado de columnas */
        wattron(panel->win, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
        mvwprintw(panel->win, 1, 2, "%-8s %-*s", "PID", inner_w - 10, "NOMBRE");
        wattroff(panel->win, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);

        /* Dibujar entradas con scroll */
        int visible_rows = inner_h - 1; /* -1 por el encabezado */
        for (row = 0; row < visible_rows && (scroll_offset + row) < list->count; row++) {
            const ProcessEntry *e = &list->entries[scroll_offset + row];

            wattron(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
            mvwprintw(panel->win, row + 2, 2, "%-8d %-*.*s",
                      e->pid,
                      inner_w - 10,
                      inner_w - 10,
                      e->name);
            wattroff(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
        }
    }

    wrefresh(panel->win);
}
