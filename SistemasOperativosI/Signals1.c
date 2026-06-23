#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void atrapa(int);

int main(){
    int i;
    for(i=1; i<=64; i++)
        signal(i, atrapa);
    printf("Identificativo de proceso: %d\n", getpid());
    pause();
    printf("Continuando despues de una captura...\n");
}

void atrapa(int sig){
    signal(sig, atrapa);
    printf("Recibida la señal: %d\n", sig);
}