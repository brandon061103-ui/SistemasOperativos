#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int id;
    system("clear");
    fprintf(stderr, "1.-Inicia Programa con Proceso Padre PID: %ld\n", (long)getpid());
    id = fork();
    if (id == 0) {
        fprintf(stderr, "\n2.-He creado el Proceso Hijo PID %ld, Mi padre es PID: %ld\n", (long)getpid(), (long)getppid());
        sleep(1);
        printf("4.-Hijo termina...bye...\n");
    } else {
        sleep(1);
        fprintf(stderr, "\n3.-Soy El Padre y Continuo el Proceso: %ld\n", (long)getpid());
        sleep(1);
    }
    fprintf(stderr, "\n\n5.-Termino mi hijo y yo tambien termino el Proceso %ld\n", (long)getpid());
    exit(0);
}