#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
    int id;
    char *buf_sgtmemc;

    if (argc != 2){
        puts("Modo de empleo: shmat <identificador>");
        exit(-1);
    }
    id=atoi(argv[1]);
    if((buf_sgtmemc=shmat(id,0,0))<(char *)0){
        perror("shmat");
        exit(-1);
    }
    printf("El segmento ha sido anclado en %p\n", buf_sgtmemc);
    system("ipcs -m");
    exit(0);
}