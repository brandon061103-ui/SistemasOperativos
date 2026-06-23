#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define NUM_THREADS 5

void *ImprimeSaludo(void *ThreadID) {
    long tID;
    tID = (long)ThreadID;
    printf("Hola esto se ejecuta en el hilo #%ld\n", tID);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    pthread_t threads[NUM_THREADS];
    int rc;
    long t;
    for (t = 0; t < NUM_THREADS; t++) {
        printf("En Funcion Main: Creando el Hilo %ld\n",t);
        rc = pthread_create(&threads[t], NULL, ImprimeSaludo, (void *)t);
        if (rc) {
            printf("ERROR, return code from pthread_create() is %d\n", rc);
            exit(-1);
        }
    }
    pthread_exit(NULL);
}