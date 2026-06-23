#include <sys/types.h>
#include <wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    pid_t idProceso;
    int estadoHijo;
    int descriptorTuberia[2];
    char Buffer[100];
    if(pipe (descriptorTuberia) == -1){
        perror("No se pudo crear la tuberia");
        exit(-1);
    }
    idProceso = fork();
    if(idProceso == -1){
        perror("No se puede crear el proceso");
        exit(-1);
    }
    if(idProceso == 0){
        close(descriptorTuberia[1]);
        read(descriptorTuberia[0], Buffer, 5);
        printf("Proceso Hijo: Recibió el msg \"%s\"\n", Buffer);
        exit(0);
    }
    if(idProceso > 0){
        close(descriptorTuberia[0]);
        printf("Proceso Padre : Envía el msg: \"Hola\"\n");
        strcpy(Buffer, "Hola");
        write(descriptorTuberia[1], Buffer, strlen(Buffer) + 1);
        wait(&estadoHijo);
        exit(0);
    }
}