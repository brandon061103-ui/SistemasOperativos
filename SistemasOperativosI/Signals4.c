#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    int pid, i=10;
    if ((pid=fork()) == 0) {
        while(i!=0){
            printf("HIJO, PID = %d\n", getpid());
            i--;
            sleep(1);
        }
    }
    else{
        sleep(10);
        printf("PADRE. Terminación del proceso %d\n", getpid());
        kill(pid, SIGCLD); //SIGTERM, SIGSTOP, SIGABRT, SIGQUIT, SIGCLD
    }
    exit(0);
}
root@BranPeubipi:~/PracticasSO/Ejercicios12Marzo2025# cat Signals4.c
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(){
    int pid, i=10;
    if ((pid=fork()) == 0) {
        while(i!=0){
            printf("HIJO, PID = %d\n", getpid());
            i--;
            sleep(1);
        }
    }
    else{
        sleep(10);
        printf("PADRE. Terminación del proceso %d\n", getpid());
        kill(pid, SIGCLD); //SIGTERM, SIGSTOP, SIGABRT, SIGQUIT, SIGCLD
    }
    exit(0);
}