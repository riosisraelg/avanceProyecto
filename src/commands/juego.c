#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    int secreto, num;
    srand(time(NULL));
    secreto = rand() % 10 + 1;

    printf("--- JUEGO: Adivina el numero (1-10) ---\n");
    printf("Tienes 3 intentos.\n");

    for (int i = 1; i <= 3; i++) {
        printf("Intento %d: ", i);
        if (scanf("%d", &num) != 1) break;

        if (num == secreto) {
            printf("¡Ganaste! El numero era %d.\n", secreto);
            return 0;
        } else if (num < secreto) printf("Mas alto...\n");
        else printf("Mas bajo...\n");
    }

    printf("Perdiste. El numero era %d.\n", secreto);
    return 0;
}