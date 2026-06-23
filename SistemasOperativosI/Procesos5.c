#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t Proceso;
    int Estado;

    system("clear");
    if ((Proceso = fork()) == -1) {
        fprintf(stderr, "\nFork Fallo");
        exit(1);
    } else if (Proceso == 0) {
        fprintf(stderr, "\nSoy Proceso Hijo PID: %ld.", (long)getpid());
    } else {
        if (wait(&Estado) != Proceso) {
            fprintf(stderr, "\nUna señal debió interrumpir la espera de la finalización");
        } else {
            fprintf(stderr, "\nSoy el proceso Padre PID: %ld y Proceso Hijo PID: %ld \n", (long)getpid(), (long)Proceso);
        }
    }
    exit(0);
}