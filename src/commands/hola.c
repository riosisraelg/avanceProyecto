#include <stdio.h>
#include <unistd.h>

int main() {
    char *msg = "Hola Mundo\n";
    for (int i = 0; msg[i] != '\0'; i++) {
        printf("%c", msg[i]);
        fflush(stdout);
        sleep(10);
    }
    return 0;
}