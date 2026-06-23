#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/*Crear un semaforo*/

int main(){
    int idsem;
    int totalsem=1;
    int indica=0666;
    struct sembuf buf;

    /*Crear semaforo con derechos de lectura/modificacion para todos los usuarios*/
    idsem=semget(IPC_PRIVATE, totalsem, indica);
    if(idsem < 0){
        perror("segment");
        exit(-1);
    }
    printf("Semaforo creado: %d\n", idsem);
    buf.sem_num=0;
    buf.sem_op=1;
    buf.sem_flg=IPC_NOWAIT;

    if((semop(idsem, &buf, totalsem))<0){
        perror("semop");
        exit(-1);
    }
    system("ipcs -s");
    exit(-1);
}