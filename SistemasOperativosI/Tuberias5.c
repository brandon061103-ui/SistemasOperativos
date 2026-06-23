#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>

int main(){
    int fd;
    int len;
    char buf[PIPE_BUF];
    time_t tp;
    mode_t modo=0666;

    printf("I am %d\n",getpid());
    if ((fd=open("fifos1",O_WRONLY))<0){
        perror("open");
        exit(0);
    }
    while (1){
       time(&tp);
       len = sprintf(buf,"wrfifo %d sends: %s",getpid(),ctime(&tp));
       if ((write(fd,buf,len+1))<0){
           perror ("write");
           close(fd);
           exit(0);
       }
       sleep(2);
    }
    close(fd);
    exit(0);
}