#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

void *funcionThread(void *parametro);

int contador = 0;

int main(){
    pthread_t IDHilo;
    int error;

    error = pthread_create (&IDHilo, NULL, funcionThread, NULL);
    if(error != 0){
        perror("No puedo crear thread");
        exit(-1);
    }
    while(contador < 10){
        contador++;
        printf("\n Incrementa proceso Padre %d\n", contador);
        sleep(1);
    }
    exit(0);
}

void *funcionThread(void *parametro){
    while(contador < 10){
        sleep(1);
        contador++;
        printf("\n Incremeneta el proceso Hijo : %d\n", contador);
        sleep(1);
    }
    exit(0);
}