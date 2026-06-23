#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void atrapa(int);

int main(){
    if(signal(SIGINT, atrapa)==SIG_ERR){
        perror("Signal");
        exit(-1);
    }
    printf("En espera de Ctrl-C\n");
    pause();
}

void atrapa(int sig)
{
    printf("\nSeñal numero %d recibida\n", sig);
}