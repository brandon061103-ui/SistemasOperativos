#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>

jmp_buf entorno;
void atrapa(int);

int main(){
    int valor;
    printf("longjmp y setjmp son una forma de simular el \'goto\'\n\n");
    signal(SIGUSR1, atrapa);
    valor=setjmp(entorno);
    if(valor == 0){
        printf("Inicia el punto de interrupción del proceso.\n");
        sleep(1);
    }
    printf("Regresé del Punto de interrupción del proceso\n");
}

void atrapa(int sig){
    signal(SIGUSR1, atrapa);
    longjmp(entorno, sig);
}