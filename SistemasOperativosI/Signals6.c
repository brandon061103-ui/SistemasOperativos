#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 256

int pid_emisor, pid_receptor;

void enviar(int sig) {
    char str[MAX];
    FILE *fp;
    printf("PROCESO EMISOR. MENSAJE: ");
    if (fgets(str, MAX, stdin) != NULL) {
        if ((fp = fopen("buzon", "w")) == NULL) {
            perror("Enviar");
            kill(pid_receptor, SIGTERM);
        }
        fputs(str, fp);
        fclose(fp);
        signal(SIGUSR1, enviar);
        kill(pid_receptor, SIGUSR1);
    } else {
        printf("\n");
        kill(pid_receptor, SIGTERM);
        exit(0);
    }
}

void recibir(int sig) {
    char str[MAX];
    FILE *fp;
    if ((fp = fopen("buzon", "r")) == NULL) {
        perror("Recibir");
        kill(pid_receptor, SIGTERM);
    }
    fgets(str, MAX, fp);
    fclose(fp);
    printf("PROCESO RECEPTOR. MENSAJE: %s\n", str);
    signal(SIGUSR1, recibir);
    kill(pid_emisor, SIGUSR1);
}

int main() {
    pid_emisor = getpid();
    if ((pid_receptor = fork()) == -1) {
        perror("fork");
        exit(-1);
    } else if (pid_receptor == 0) {
        signal(SIGUSR1, recibir);
        while (1)
            pause();
    } else {
        printf("Inicializando...\n");
        sleep(2);
        enviar(SIGUSR1);
        while (1)
            pause();
    }
    exit(0);
}