#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void atrapa(int);

int main(){
    int i;
    signal(SIGALRM, atrapa);
    printf("Identificativo de proceso %d\n", getpid());;
    alarm(5);
    pause();
    alarm(3);
    pause();
    for(i=1; i<10; i++){
        alarm(1);
        pause();
    }
    return 0;
}

void atrapa(int sig){
    signal(sig, atrapa);
    printf("RIIIIIING!\n");
}