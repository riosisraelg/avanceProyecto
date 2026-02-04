#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int total = 0, carta;
    char opcion;
    srand(time(NULL));

    printf("--- Partida de 21 ---\n");

    while (total < 21) {
        carta = (rand() % 10) + 1;
        total += carta;
        printf("Carta: %d | Total: %d\n", carta, total);

        if (total >= 21) break;

        printf("¿Otra carta? (s/n): ");
        if (scanf(" %c", &opcion) != 1 || opcion == 'n') break;
    }

    if (total == 21) printf("¡21! ¡Ganaste!\n");
    else if (total > 21) printf("Te pasaste. Perdiste.\n");
    else printf("Te plantaste con %d.\n", total);

    return 0;
}