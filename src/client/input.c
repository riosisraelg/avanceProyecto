#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "input.h"
#include "colors.h"

/*
 * Inicializa la línea de entrada con buffer vacío y cursor en posición 0.
 */
void input_init(InputLine *line)
{
    if (!line) {
        return;
    }
    memset(line->buffer, 0, INPUT_BUF_SIZE);
    line->cursor_pos = 0;
    line->length = 0;
}

/*
 * Limpia la línea de entrada (resetea buffer, cursor y longitud).
 */
void input_clear(InputLine *line)
{
    if (!line) {
        return;
    }
    memset(line->buffer, 0, INPUT_BUF_SIZE);
    line->cursor_pos = 0;
    line->length = 0;
}

/*
 * Normaliza una cadena a mayúsculas (in-place).
 * Reutiliza la lógica de to_uppercase del main.c original.
 */
void input_to_uppercase(char *str)
{
    int i;
    if (!str) {
        return;
    }
    for (i = 0; str[i]; i++) {
        str[i] = (char)toupper((unsigned char)str[i]);
    }
}

/*
 * Genera el prompt con formato "remote@{IP}:{PUERTO}> ".
 * Función pura: no depende de ncurses, facilita testing.
 */
void input_format_prompt(char *buf, int buf_size, const char *ip, int port)
{
    if (!buf || buf_size <= 0) {
        return;
    }
    if (!ip) {
        ip = "0.0.0.0";
    }
    snprintf(buf, buf_size, "remote@%s:%d> ", ip, port);
}

/*
 * Procesa una tecla de entrada.
 *
 * - Caracteres imprimibles: se insertan en la posición del cursor.
 * - Backspace (KEY_BACKSPACE, 127, '\b'): borra el carácter anterior.
 * - Enter ('\n', '\r', KEY_ENTER): si el buffer no está vacío, normaliza
 *   el comando a mayúsculas y retorna 1 (comando listo). Si está vacío,
 *   retorna 0 sin marcar comando listo (Requisito 5.5).
 *
 * Retorna 1 si se presionó Enter con buffer no vacío, 0 en otro caso.
 */
int input_handle_key(InputLine *line, int ch)
{
    if (!line) {
        return 0;
    }

    /* Enter */
    if (ch == '\n' || ch == '\r' || ch == KEY_ENTER) {
        /* Ignorar Enter con línea vacía (Requisito 5.5) */
        if (line->length == 0) {
            return 0;
        }
        /* Normalizar comando a mayúsculas antes de marcar como listo */
        input_to_uppercase(line->buffer);
        return 1;
    }

    /* Backspace */
    if (ch == KEY_BACKSPACE || ch == 127 || ch == '\b') {
        if (line->cursor_pos > 0) {
            /* Mover caracteres a la izquierda desde cursor_pos */
            memmove(&line->buffer[line->cursor_pos - 1],
                    &line->buffer[line->cursor_pos],
                    line->length - line->cursor_pos);
            line->cursor_pos--;
            line->length--;
            line->buffer[line->length] = '\0';
        }
        return 0;
    }

    /* Caracteres imprimibles */
    if (isprint(ch) && line->length < INPUT_BUF_SIZE - 1) {
        /* Insertar en la posición del cursor */
        memmove(&line->buffer[line->cursor_pos + 1],
                &line->buffer[line->cursor_pos],
                line->length - line->cursor_pos);
        line->buffer[line->cursor_pos] = (char)ch;
        line->cursor_pos++;
        line->length++;
        line->buffer[line->length] = '\0';
        return 0;
    }

    return 0;
}

/*
 * Renderiza la línea de entrada en el Panel_Entrada.
 * Muestra el prompt seguido del contenido del buffer.
 * El cursor se posiciona después del prompt + cursor_pos.
 */
void input_render(InputLine *line, Panel *panel, const char *prompt)
{
    int prompt_len;

    if (!line || !panel || !panel->win) {
        return;
    }

    /* Limpiar el área interior del panel (fila 1, dentro del borde) */
    wmove(panel->win, 1, 1);
    wclrtoeol(panel->win);

    /* Redibujar el borde derecho que wclrtoeol pudo borrar */
    mvwaddch(panel->win, 1, panel->width - 1, ACS_VLINE);

    /* Mostrar prompt con color de texto */
    wattron(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
    mvwprintw(panel->win, 1, 1, "%s", prompt ? prompt : "> ");
    wattroff(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));

    /* Mostrar buffer */
    prompt_len = prompt ? (int)strlen(prompt) : 2;
    wattron(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));
    mvwprintw(panel->win, 1, 1 + prompt_len, "%s", line->buffer);
    wattroff(panel->win, COLOR_PAIR(COLOR_PAIR_TEXT));

    /* Posicionar cursor */
    wmove(panel->win, 1, 1 + prompt_len + line->cursor_pos);

    wrefresh(panel->win);
}
