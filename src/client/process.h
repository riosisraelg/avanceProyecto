#ifndef PROCESS_H
#define PROCESS_H

#include "panels.h"

#define PROC_NAME_SIZE 256

typedef struct {
    int pid;
    char name[PROC_NAME_SIZE];
} ProcessEntry;

typedef struct {
    ProcessEntry *entries;
    int count;
    int capacity;
} ProcessList;

/*
 * Parsea la respuesta cruda del servidor (texto de `ps -e -o pid,comm`)
 * en una ProcessList. La primera línea (encabezado) se omite.
 *
 * Retorna 0 si OK, -1 en error.
 * Esta función es pura (sin dependencia de ncurses).
 */
int process_list_parse(const char *raw_response, ProcessList *list);

/*
 * Libera la memoria de la lista de procesos.
 */
void process_list_free(ProcessList *list);

/*
 * Renderiza la lista de procesos en el Panel_Procesos con columnas alineadas.
 * Muestra "Sin procesos activos" centrado si la lista está vacía.
 *
 * scroll_offset: offset de scroll actual para la vista.
 */
void process_list_render(const ProcessList *list, Panel *panel, int scroll_offset);

#endif /* PROCESS_H */
