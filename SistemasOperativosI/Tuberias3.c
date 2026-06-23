#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 256

int main(){
    int a[2];
    int b[2];
    int pid;
    char mensaje[MAX];

    if(pipe(a) == -1 || pipe(b) == -1){
        perror("Error al crear los pipe");
        exit(-1);
    }
    if(pid = fork() == -1){
        perror("Fork");
        exit(-1);
    }
    else if (pid == 0){
        close(a[1]);
        close(b[0]);
        while(read(a[0], mensaje, MAX) > 0 && strcmp(mensaje, "FIN") != 0){
            printf("PROCESO RECEPTOR. MENSAJE %s\n", mensaje);
            strcpy(mensaje, "LISTO");
            printf("VA MENSAJE: %s\n", mensaje);
            write(b[1], mensaje, strlen(mensaje)+1);
        }
    close(a[0]);
    close(b[1]);
    }
    else{
        close(a[0]);
        close(b[1]);
        while(printf("PROCESO EMISOR. MENSAJE: ")!=0 && gets(mensaje) != NULL && write(a[1], mensaje, strlen(mensaje)+1) > 0 && strcmp(mensaje, "FIN") != 0)
        do{
            read(b[0], mensaje, MAX);
        }
        while(strcmp(mensaje, "LISTO") != 0);
        close(a[1]);
        close(b[0]);
        exit(0);
    }
}