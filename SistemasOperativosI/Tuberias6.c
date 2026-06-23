#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>

int main(){
    int fd;
    int len;
    char buf[PIPE_BUF];
    mode_t modo=0666;
    if ((mkfifo("TuberiaNuevo",modo)) < 0){
        printf("error");
        perror("mkfifo");
        exit(0);
    }

    if ((fd=open("fifos1",O_RDONLY))<0){
        perror("open");
        exit(0);
    }

    while ((len=read(fd,buf,PIPE_BUF-1))>0)
        printf("rdfifo read: %s",buf);
    close(fd);
    exit(0);
}