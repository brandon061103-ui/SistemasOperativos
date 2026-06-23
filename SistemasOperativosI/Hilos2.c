#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *funcionThread(void *parametro);

int main() {
    pthread_t IDHilo;
    pthread_attr_t atributos;
    int error;

    char *valorDevuelto = NULL;

    pthread_attr_init(&atributos);
    pthread_attr_setdetachstate(&atributos, PTHREAD_CREATE_JOINABLE);

    error = pthread_create(&IDHilo, &atributos, funcionThread, NULL);

    if (error != 0) {
        perror("No puedo crear thread");
        exit(-1);
    }

    printf("Proceso Padre: Espero al thread a que termine su ejecucion\n");
    pthread_join(IDHilo, (void **)&valorDevuelto);
    printf("Proceso Padre: Ya se ha terminado el hilo thread\n");

    printf("Proceso Padre: devuelve \"%s\"\n", valorDevuelto);
    return 0;
}

void *funcionThread(void *parametro) {
    printf("Proceso Hijo: Espero 1 segundo\n");
    sleep(1);
    printf("Hijo: Termino\n");
    pthread_exit((void*)"Ya esta");
}