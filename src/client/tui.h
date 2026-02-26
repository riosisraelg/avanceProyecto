#ifndef TUI_H
#define TUI_H

#include "panels.h"
#include "input.h"
#include "net.h"
#include "process.h"

typedef struct {
    TUILayout *layout;
    InputLine input_line;
    SOCKET sock;
    char server_ip[46];
    int server_port;
    int running;
    char status_msg[256];
    char *proc_lines;       /* Buffer crudo con la lista de procesos */
    int proc_scroll_offset; /* Offset de scroll en Panel_Procesos */
    int proc_line_count;    /* Número total de líneas de procesos */
    ProcessList proc_list;  /* Lista estructurada de procesos */
} TUIState;

/* Inicializa ncurses, colores, paneles. Retorna el estado de la TUI. */
TUIState *tui_init(void);

/* Muestra el diálogo de conexión inicial. Retorna 0 si se conectó, -1 si el usuario canceló. */
int tui_connection_dialog(TUIState *state);

/* Ejecuta el bucle principal de la TUI (input, render, network). */
void tui_run(TUIState *state);

/* Limpia ncurses, libera memoria, restaura terminal. */
void tui_shutdown(TUIState *state);

#endif /* TUI_H */
