#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>

#define SEM_HIJO 0
#define SEM_PADRE 1

int main(int argc, char *argv[]) {
    int i = 10, semid, pid;
    struct sembuf opera;
    key_t llave;

    /* Petición de un identificador con dos semáforos */
    llave = ftok(argv[0], 'K');

    if ((semid = semget(llave, 2, IPC_CREAT | 0600)) == -1) {
        perror("semget");
        exit(-1);
    }

    /* Inicialización de dos semáforos */
    /* Se cierra el semáforo del proceso hijo */
    semctl(semid, SEM_HIJO, SETVAL, 0);

    /* Abrir el semáforo del padre */
    semctl(semid, SEM_PADRE, SETVAL, 1);
    system("ipcs -a");

    /* Creación del proceso hijo */
    if ((pid = fork()) == -1) {
        perror("fork");
        exit(-1);
    } else if (pid == 0) {
        /* El proceso hijo espera hasta que el padre termine */
        opera.sem_num = SEM_HIJO;
        opera.sem_op = -1;
        opera.sem_flg = 0;
        semop(semid, &opera, 1);

        i = 10;  // Reiniciar el contador para el hijo
        while (i) {
            printf("Proceso Hijo: %d\n", i--);
        }
        /* Borrando semáforo */
        semctl(semid, SEM_HIJO, IPC_RMID, 0);
    } else {
        opera.sem_flg = 0;
        while (i) {
            /* Cierra el semáforo padre */
            opera.sem_num = SEM_PADRE;
            opera.sem_op = -1;
            semop(semid, &opera, 1);
            printf("Proceso Padre: %d\n", i--);
        }
        /* Abre el semáforo del hijo */
        opera.sem_num = SEM_HIJO;
        opera.sem_op = 1;
        semop(semid, &opera, 1);

        /* Borrado del semáforo */
        semctl(semid, SEM_PADRE, IPC_RMID, 0);
    }
}