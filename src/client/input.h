#ifndef INPUT_H
#define INPUT_H

#include "panels.h"

#define INPUT_BUF_SIZE 256

typedef struct {
    char buffer[INPUT_BUF_SIZE];
    int cursor_pos;
    int length;
} InputLine;

/* Inicializa la línea de entrada. */
void input_init(InputLine *line);

/*
 * Procesa una tecla.
 * Retorna 1 si se presionó Enter con buffer no vacío (comando listo), 0 si no.
 */
int input_handle_key(InputLine *line, int ch);

/* Limpia la línea de entrada después de enviar un comando. */
void input_clear(InputLine *line);

/* Renderiza la línea de entrada en el Panel_Entrada. */
void input_render(InputLine *line, Panel *panel, const char *prompt);

/*
 * Genera el prompt con formato "remote@{IP}:{PUERTO}> ".
 * Función pura expuesta para testing (no depende de ncurses).
 *
 * Parámetros:
 *   buf      - buffer de destino
 *   buf_size - tamaño del buffer
 *   ip       - dirección IP del servidor
 *   port     - puerto del servidor
 */
void input_format_prompt(char *buf, int buf_size, const char *ip, int port);

/*
 * Normaliza una cadena a mayúsculas (in-place).
 * Reutiliza la lógica de to_uppercase del main.c original.
 * Función pura expuesta para testing.
 */
void input_to_uppercase(char *str);

#endif /* INPUT_H */
