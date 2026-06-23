#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    int i;
    system("clear");
    for (i = 0; i < 4; i++) {
        if (fork())
            break;
        fprintf(stderr, "\nProceso %d con PID: %ld\tPadre PID: %ld", i, (long)getpid(), (long)getppid());
    }
    fprintf(stderr, "\n\nFinalizado Proceso PID: %ld\n", (long)getpid());
    sleep(1);
    exit(0);
}