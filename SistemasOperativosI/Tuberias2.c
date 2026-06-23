#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256

int main() {
    int Tuberia[2];
    int pid, conta;
    char mensaje[MAX];

    if (pipe(Tuberia) == -1) {
        perror("Pipe");
        exit(-1);
    }

    if ((pid = fork()) == -1) {
        perror("Fork");
        exit(-1);
    } else if (pid == 0) {
        conta=1;
        while (read(Tuberia[0], mensaje, MAX) > 0 && strcmp(mensaje, "FIN") != 0) {
            printf("PROCESO RECEPTOR. MENSAJE No. %d: %s\n", conta++, mensaje);
        }
        close(Tuberia[0]);
        close(Tuberia[1]);
        exit(0);
    } else {
        while (printf("PROCESO EMISOR DE MENSAJE: ") != 0 && gets(mensaje) != NULL && write(Tuberia[1], mensaje, strlen(mensaje) + 1) > 0 && strcmp(mensaje, "FIN\n") != 0) {
        }
        close(Tuberia[0]);
        close(Tuberia[1]);
        exit(0);
    }
}