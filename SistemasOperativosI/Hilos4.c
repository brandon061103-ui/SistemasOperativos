#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int cont = 0;

void *Incrementar(void *param) {
    int i, n = atoi(param);
    for (i = 0; i < n; i++)
        cont++;
    pthread_exit(0);
}

void *Decrementar(void *param) {
    int i, n = atoi(param);
    for (i = 0; i < n; i++)
        cont--;
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    pthread_t tidi, tidd;
    pthread_attr_t attr;

    if (argc < 2) {
        fprintf(stderr, "pthread1 <valor entero>\n");
        return -1;
    }

    pthread_attr_init(&attr);
    pthread_create(&tidi, &attr, Incrementar, argv[1]);
    pthread_create(&tidd, &attr, Decrementar, argv[1]);
    printf("Contador 1 = %d\n", cont);
    pthread_join(tidi, NULL);

    printf("Contador 2 = %d\n", cont);
    pthread_join(tidd, NULL);
    printf("Contador 2 = %d\n", cont);
    return 0;
}