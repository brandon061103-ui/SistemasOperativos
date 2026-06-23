#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    pid_t pid_hijo;
    char *parametros[3];
    if((pid_hijo = fork())==-1){
        printf("Error al crear el proceso hijo\n");
        exit(-1);
    }
    else if (pid_hijo == 0){
        parametros[0] = "ls";
        parametros[1] = "-1";
        parametros[2] = NULL;
        execvp("ls", parametros);
    }
    else
    exit(0);
}