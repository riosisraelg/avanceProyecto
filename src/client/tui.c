#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "tui.h"
#include "colors.h"
#include "curses_compat.h"

TUIState *tui_init(void) {
    TUIState *state = calloc(1, sizeof(TUIState));
    if (!state)
        return NULL;

    /* Inicializar ncurses */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);

    /* Inicializar paleta de grises */
    colors_init();

    /* Crear paneles */
    state->layout = panels_create();
    if (!state->layout) {
        endwin();
        free(state);
        return NULL;
    }

    /* Inicializar línea de entrada */
    input_init(&state->input_line);

    /* Estado inicial */
    state->running = 1;
    state->sock = INVALID_SOCKET;
    state->server_ip[0] = '\0';
    state->server_port = 0;
    state->status_msg[0] = '\0';
    state->proc_lines = NULL;
    state->proc_scroll_offset = 0;
    state->proc_line_count = 0;
    state->proc_list.entries  = NULL;
    state->proc_list.count    = 0;
    state->proc_list.capacity = 0;

    return state;
}

int tui_connection_dialog(TUIState *state) {
    if (!state)
        return -1;

    /* Dimensiones del diálogo */
    int dw = 50;
    int dh = 12;
    int starty = (LINES - dh) / 2;
    int startx = (COLS - dw) / 2;

    /* Clamp para terminales pequeñas */
    if (starty < 0) starty = 0;
    if (startx < 0) startx = 0;
    if (dw > COLS) dw = COLS;
    if (dh > LINES) dh = LINES;

    WINDOW *dwin = newwin(dh, dw, starty, startx);
    if (!dwin)
        return -1;

    keypad(dwin, TRUE);
    nodelay(dwin, FALSE); /* Blocking input for dialog */

    /* Buffers con valores por defecto */
    char ip_buf[46];
    char port_buf[8];
    strncpy(ip_buf, "127.0.0.1", sizeof(ip_buf) - 1);
    ip_buf[sizeof(ip_buf) - 1] = '\0';
    strncpy(port_buf, "5002", sizeof(port_buf) - 1);
    port_buf[sizeof(port_buf) - 1] = '\0';

    int field = 0; /* 0 = IP, 1 = Puerto */
    char error_msg[128] = "";

    for (;;) {
        /* Dibujar diálogo */
        werase(dwin);
        wattron(dwin, COLOR_PAIR(COLOR_PAIR_BORDER));
        box(dwin, 0, 0);
        wattroff(dwin, COLOR_PAIR(COLOR_PAIR_BORDER));

        /* Título centrado */
        const char *title = " Conexion al Servidor ";
        int title_x = (dw - (int)strlen(title)) / 2;
        if (title_x < 1) title_x = 1;
        wattron(dwin, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);
        mvwprintw(dwin, 0, title_x, "%s", title);
        wattroff(dwin, COLOR_PAIR(COLOR_PAIR_HEADER) | A_BOLD);

        /* Campo IP */
        wattron(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(dwin, 2, 3, "IP del servidor:");
        wattroff(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));

        if (field == 0)
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_SELECTED));
        else
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(dwin, 3, 3, "  %-40s", ip_buf);
        if (field == 0)
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_SELECTED));
        else
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));

        /* Campo Puerto */
        wattron(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(dwin, 5, 3, "Puerto:");
        wattroff(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));

        if (field == 1)
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_SELECTED));
        else
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(dwin, 6, 3, "  %-40s", port_buf);
        if (field == 1)
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_SELECTED));
        else
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));

        /* Mensaje de error si existe */
        if (error_msg[0] != '\0') {
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_ERROR) | A_BOLD);
            mvwprintw(dwin, 8, 3, "%.44s", error_msg);
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_ERROR) | A_BOLD);
        }

        /* Instrucciones */
        wattron(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));
        mvwprintw(dwin, dh - 2, 3, "Enter: conectar  Tab: campo  ESC/q: salir");
        wattroff(dwin, COLOR_PAIR(COLOR_PAIR_TEXT));

        wrefresh(dwin);

        /* Leer tecla */
        int ch = wgetch(dwin);

        if (ch == 27 || ch == 'q') {
            /* ESC o q → cancelar */
            delwin(dwin);
            return -1;
        }

        if (ch == '\t' || ch == KEY_DOWN || ch == KEY_UP) {
            /* Cambiar campo */
            field = (field == 0) ? 1 : 0;
            continue;
        }

        if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
            /* Intentar conexión */
            int port = atoi(port_buf);
            if (port <= 0 || port > 65535) {
                snprintf(error_msg, sizeof(error_msg),
                         "Puerto invalido: %s", port_buf);
                continue;
            }
            if (ip_buf[0] == '\0') {
                snprintf(error_msg, sizeof(error_msg),
                         "IP no puede estar vacia");
                continue;
            }

            /* Mostrar mensaje de conexión */
            wattron(dwin, COLOR_PAIR(COLOR_PAIR_HEADER));
            mvwprintw(dwin, 8, 3, "%-44s", "Conectando...");
            wattroff(dwin, COLOR_PAIR(COLOR_PAIR_HEADER));
            wrefresh(dwin);

            SOCKET sock = net_connect(ip_buf, port);
            if (sock == INVALID_SOCKET) {
                snprintf(error_msg, sizeof(error_msg),
                         "Error: no se pudo conectar a %s:%d", ip_buf, port);
                continue; /* Permite reintentar */
            }

            /* Conexión exitosa — guardar estado */
            state->sock = sock;
            strncpy(state->server_ip, ip_buf, sizeof(state->server_ip) - 1);
            state->server_ip[sizeof(state->server_ip) - 1] = '\0';
            state->server_port = port;
            snprintf(state->status_msg, sizeof(state->status_msg),
                     "Conectado a %s:%d", ip_buf, port);

            /* Enviar LIST automáticamente */
            net_send(sock, "LIST\n");

            /* Recibir respuesta con timeout breve usando select */
            char recv_buf[NET_BUFFER_SIZE];
            fd_set rfds;
            struct timeval tv;
            FD_ZERO(&rfds);
            FD_SET(sock, &rfds);
            tv.tv_sec = 3;
            tv.tv_usec = 0;

            int ready = select((int)sock + 1, &rfds, NULL, NULL, &tv);
            if (ready > 0) {
                int n = recv(sock, recv_buf, NET_BUFFER_SIZE - 1, 0);
                if (n > 0) {
                    recv_buf[n] = '\0';
                    /* Almacenar la lista cruda de procesos */
                    if (state->proc_lines)
                        free(state->proc_lines);
                    state->proc_lines = malloc((size_t)n + 1);
                    if (state->proc_lines) {
                        memcpy(state->proc_lines, recv_buf, (size_t)n + 1);
                        /* Contar líneas */
                        state->proc_line_count = 0;
                        for (int i = 0; i < n; i++) {
                            if (recv_buf[i] == '\n')
                                state->proc_line_count++;
                        }
                        /* Si no termina en newline, contar la última línea */
                        if (n > 0 && recv_buf[n - 1] != '\n')
                            state->proc_line_count++;
                    }

                    /* Parsear en la lista estructurada */
                    process_list_free(&state->proc_list);
                    process_list_parse(recv_buf, &state->proc_list);
                }
            }

            state->proc_scroll_offset = 0;

            delwin(dwin);
            return 0;
        }

        /* Edición del campo activo */
        char *buf;
        int max_len;
        if (field == 0) {
            buf = ip_buf;
            max_len = (int)sizeof(ip_buf) - 1;
        } else {
            buf = port_buf;
            max_len = (int)sizeof(port_buf) - 1;
        }

        int len = (int)strlen(buf);

        if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (len > 0)
                buf[len - 1] = '\0';
        } else if (ch >= 32 && ch <= 126 && len < max_len) {
            buf[len] = (char)ch;
            buf[len + 1] = '\0';
        }

        /* Limpiar error al editar */
        error_msg[0] = '\0';
    }
}

void tui_shutdown(TUIState *state) {
    if (!state)
        return;

    /* Destruir paneles */
    if (state->layout)
        panels_destroy(state->layout);

    /* Restaurar terminal */
    endwin();

    /* Cerrar conexión si está activa */
    if (state->sock != INVALID_SOCKET)
        net_close(state->sock);

    /* Liberar buffer de procesos */
    if (state->proc_lines)
        free(state->proc_lines);

    /* Liberar lista estructurada de procesos */
    process_list_free(&state->proc_list);

    free(state);
}

/*
 * Renderiza la Barra_Estado con el mensaje actual.
 */
static void render_status_bar(TUIState *state)
{
    Panel *sp = &state->layout->status;
    int inner_w;

    if (!sp->win)
        return;

    inner_w = sp->width - 2;
    if (inner_w <= 0)
        return;

    /* Limpiar interior */
    wmove(sp->win, 1, 1);
    {
        int i;
        for (i = 0; i < inner_w; i++)
            waddch(sp->win, ' ');
    }

    /* Mostrar mensaje de estado */
    wattron(sp->win, COLOR_PAIR(COLOR_PAIR_HEADER));
    mvwprintw(sp->win, 1, 1, "%.*s", inner_w, state->status_msg);
    wattroff(sp->win, COLOR_PAIR(COLOR_PAIR_HEADER));

    wrefresh(sp->win);
}

/*
 * Envía un comando al servidor, recibe la respuesta y actualiza el estado.
 * Retorna 1 si el comando fue EXIT (señal de salir), 0 en otro caso.
 */
static int handle_command(TUIState *state, const char *cmd)
{
    char send_buf[INPUT_BUF_SIZE + 2];
    char recv_buf[NET_BUFFER_SIZE];
    int n;

    /* Verificar si es EXIT */
    if (strcmp(cmd, "EXIT") == 0) {
        snprintf(state->status_msg, sizeof(state->status_msg),
                 "Desconectando...");
        render_status_bar(state);
        net_send(state->sock, "EXIT\n");
        return 1;
    }

    /* Actualizar barra de estado */
    snprintf(state->status_msg, sizeof(state->status_msg),
             "Enviando comando...");
    render_status_bar(state);

    /* Enviar comando con newline */
    snprintf(send_buf, sizeof(send_buf), "%s\n", cmd);
    net_send(state->sock, send_buf);

    /* Recibir respuesta con timeout usando select */
    {
        fd_set rfds;
        struct timeval tv;
        FD_ZERO(&rfds);
        FD_SET(state->sock, &rfds);
        tv.tv_sec = 3;
        tv.tv_usec = 0;

        int ready = select((int)state->sock + 1, &rfds, NULL, NULL, &tv);
        if (ready > 0) {
            n = recv(state->sock, recv_buf, NET_BUFFER_SIZE - 1, 0);
            if (n > 0) {
                recv_buf[n] = '\0';

                /* Actualizar buffer crudo */
                if (state->proc_lines)
                    free(state->proc_lines);
                state->proc_lines = malloc((size_t)n + 1);
                if (state->proc_lines) {
                    memcpy(state->proc_lines, recv_buf, (size_t)n + 1);
                    state->proc_line_count = 0;
                    {
                        int i;
                        for (i = 0; i < n; i++) {
                            if (recv_buf[i] == '\n')
                                state->proc_line_count++;
                        }
                        if (n > 0 && recv_buf[n - 1] != '\n')
                            state->proc_line_count++;
                    }
                }

                /* Parsear lista estructurada */
                process_list_free(&state->proc_list);
                process_list_parse(recv_buf, &state->proc_list);
                state->proc_scroll_offset = 0;
            } else if (n == 0) {
                /* Servidor cerró la conexión */
                snprintf(state->status_msg, sizeof(state->status_msg),
                         "Conexion perdida");
                state->running = 0;
                return 0;
            }
        }
    }

    /* Restaurar mensaje de estado normal */
    snprintf(state->status_msg, sizeof(state->status_msg),
             "Conectado a %s:%d", state->server_ip, state->server_port);

    return 0;
}

void tui_run(TUIState *state)
{
    char prompt[128];
    int ch;
    int visible_h;

    if (!state || !state->layout)
        return;

    /* Generar prompt */
    input_format_prompt(prompt, sizeof(prompt),
                        state->server_ip, state->server_port);

    /* Asegurar modo no bloqueante en stdscr */
    nodelay(stdscr, TRUE);

    while (state->running) {
        /* --- Dibujar bordes y títulos --- */
        panels_draw_borders(state->layout);

        /* --- Renderizar Panel_Procesos --- */
        process_list_render(&state->proc_list, &state->layout->proc,
                            state->proc_scroll_offset);

        /* --- Renderizar Panel_Entrada --- */
        input_render(&state->input_line, &state->layout->input, prompt);

        /* --- Renderizar Barra_Estado --- */
        render_status_bar(state);

        /* --- Indicador de foco --- */
        if (state->layout->focused == 0 && state->layout->proc.win) {
            wattron(state->layout->proc.win, COLOR_PAIR(COLOR_PAIR_SELECTED));
            mvwprintw(state->layout->proc.win, 0, 1, "[*]");
            wattroff(state->layout->proc.win, COLOR_PAIR(COLOR_PAIR_SELECTED));
            wrefresh(state->layout->proc.win);
        }

        doupdate();

        /* --- Leer tecla (no bloqueante) --- */
        ch = wgetch(stdscr);

        if (ch == ERR) {
            /* Sin tecla — verificar socket para datos entrantes */
            if (state->sock != INVALID_SOCKET) {
                char async_buf[NET_BUFFER_SIZE];
                int nr = net_recv(state->sock, async_buf, NET_BUFFER_SIZE);
                if (nr > 0) {
                    /* Datos recibidos asíncronamente — actualizar lista */
                    if (state->proc_lines)
                        free(state->proc_lines);
                    state->proc_lines = malloc((size_t)nr + 1);
                    if (state->proc_lines) {
                        memcpy(state->proc_lines, async_buf, (size_t)nr + 1);
                        state->proc_line_count = 0;
                        {
                            int i;
                            for (i = 0; i < nr; i++) {
                                if (async_buf[i] == '\n')
                                    state->proc_line_count++;
                            }
                            if (nr > 0 && async_buf[nr - 1] != '\n')
                                state->proc_line_count++;
                        }
                    }
                    process_list_free(&state->proc_list);
                    process_list_parse(async_buf, &state->proc_list);
                } else if (nr < 0) {
                    /* Conexión perdida */
                    snprintf(state->status_msg, sizeof(state->status_msg),
                             "Conexion perdida");
                    state->running = 0;
                }
            }

            /* Pequeña pausa para no saturar la CPU */
            napms(30);
            continue;
        }

        /* --- Ctrl+C (valor 3 en ASCII) --- */
        if (ch == 3) {
            state->running = 0;
            break;
        }

        /* --- KEY_RESIZE: redimensionar paneles --- */
        if (ch == KEY_RESIZE) {
            panels_resize(state->layout);
            clear();
            refresh();
            continue;
        }

        /* --- Tab: cambiar foco --- */
        if (ch == '\t') {
            state->layout->focused = (state->layout->focused == 0) ? 1 : 0;
            continue;
        }

        /* --- Flechas arriba/abajo: scroll en Panel_Procesos si tiene foco --- */
        if (state->layout->focused == 0) {
            if (ch == KEY_UP) {
                visible_h = state->layout->proc.height - 3; /* -2 borde -1 header */
                state->proc_scroll_offset = scroll_clamp(
                    state->proc_scroll_offset, -1,
                    state->proc_list.count, visible_h);
                continue;
            }
            if (ch == KEY_DOWN) {
                visible_h = state->layout->proc.height - 3;
                state->proc_scroll_offset = scroll_clamp(
                    state->proc_scroll_offset, 1,
                    state->proc_list.count, visible_h);
                continue;
            }
        }

        /* --- Delegar a input_handle_key --- */
        if (input_handle_key(&state->input_line, ch)) {
            /* Enter presionado con comando listo */
            if (handle_command(state, state->input_line.buffer)) {
                /* EXIT — salir del bucle */
                state->running = 0;
            }
            input_clear(&state->input_line);
        }
    }
}


