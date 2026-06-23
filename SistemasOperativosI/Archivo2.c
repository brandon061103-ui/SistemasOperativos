#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define BYTES_BUFFER 512

int main(int argc, char *argv[]){
    int arch;
    int i, leido;
    char buffer[BYTES_BUFFER];
    arch=open(argv[1], O_RDONLY);
    if(arch<0){
        printf("Error al abrir el archivo");
        exit(-1);
    }
    while((leido=read(arch,buffer,BYTES_BUFFER))>0){
        for(i = 0; i < leido; i++){
            putchar(buffer[i]);
        }
        close(arch);
        if(leido<0){
            printf("Error al leer al archivo");
            (exit -2);
        }
    }
}
root@BranPeubipi:~/PracticasSO/Ejercicios21Febrero2025# cat Archivo2.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]){
    char Orden1[30];
    char Orden2[30];
    int i;
    for(i=1; i<argc; i++){
        sprintf(Orden1, "ls %s", argv [i]);
        sprintf(Orden2, "cat %s", argv [i]);

        system(Orden1);
        getchar();
        system(Orden2);
    }
}