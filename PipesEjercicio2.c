#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
//Proceso Receptor
//unlink(char *fifo) - Borrar una tuberia desde un programa
int main(){
    int fd, n;
    char buffer[1024];
    fd=open("mi_fifo", O_RDONLY);
    n=read(fd, buffer, sizeof(buffer));
    printf("Numero de bytes leidos %d\n", n);
    printf("Mensaje leido de la tuberia %s\n", buffer);
    close(fd);
    return 0;
}
