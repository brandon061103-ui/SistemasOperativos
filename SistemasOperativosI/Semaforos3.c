#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*Crear un semaforo*/

int main(int argc, char *argv[]){
    int idsem;

    if(argc != 2){
        puts("MODO DE EMPLEO: eliminar <idesm>");
        exit(-1);
    }

    idsem=atoi(argv[1]);

    /*Eliminar semaforo*/
    if((semctl(idsem, 0, IPC_RMID))<0){
        perror("semctl IPC_RMID");
        exit(-1);
    }
    else{
        puts("Semaforo eliminado");
        system("ipcs -s");
    }
    exit(0);
}