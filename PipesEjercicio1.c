#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
//Proceso Emisor

int main(){
    int fd;
    char buffer[]="Hola Mundo!!!";
    mkfifo("mi_fifo", 0666);
    fd=open("mi_fifo", O_WRONLY);
    write(fd, buffer, sizeof(buffer));
    close(fd);
    return 0;
}
