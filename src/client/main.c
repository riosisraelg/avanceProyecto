/**
 * main.c — Punto de entrada mínimo del cliente TUI.
 *
 * Orquestador delgado que delega toda la lógica de UI al módulo TUI
 * y toda la lógica de red al módulo net.
 */

#include "tui.h"
#include "net.h"

#include <signal.h>
#include <stdlib.h>

/* Estado global para que el handler de señales pueda acceder a él. */
static TUIState *g_state = NULL;

/**
 * Handler para SIGINT (Ctrl+C).
 * Llama a tui_shutdown() (que internamente invoca endwin() y net_close())
 * y luego limpia la plataforma de red antes de salir con código 0.
 */
static void sigint_handler(int sig) {
    (void)sig;
    if (g_state != NULL) {
        tui_shutdown(g_state);
        g_state = NULL;
    }
    net_cleanup_platform();
    exit(0);
}

int main(void) {
    if (net_init_platform() != 0) {
        return EXIT_FAILURE;
    }

    TUIState *state = tui_init();
    if (state == NULL) {
        net_cleanup_platform();
        return EXIT_FAILURE;
    }

    g_state = state;
    signal(SIGINT, sigint_handler);

    if (tui_connection_dialog(state) != 0) {
        tui_shutdown(state);
        g_state = NULL;
        net_cleanup_platform();
        return EXIT_SUCCESS;
    }

    tui_run(state);

    tui_shutdown(state);
    g_state = NULL;
    net_cleanup_platform();

    return EXIT_SUCCESS;
}
