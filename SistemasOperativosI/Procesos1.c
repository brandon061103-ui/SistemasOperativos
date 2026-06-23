#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    system("clear");
    printf("ID del proceso: %ld\n", (long)getpid());
    printf("ID del Proceso Padre: %ld\n", (long)getppid());
    printf("ID del Usuario en el Sistema: %ld\n", (long)getuid());
    printf("ID del Usuario Efectivo en el Sistema: %ld\n", (long)geteuid());
    printf("\nPulsa enter para salir...");
    getchar();
    return 0;
}