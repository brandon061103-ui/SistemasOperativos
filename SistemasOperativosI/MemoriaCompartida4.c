#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define LLAVE 234
#define MAX 10

int main() {
    int shmid, i;
    key_t key = LLAVE;
    char *vector;
    char ascii;

    /* Creación de la memoria compartida */
    shmid = shmget(key, (MAX * sizeof(char)), IPC_CREAT | 0600);
    if (shmid == -1) {
        puts("No se ha podido crear el segmento.");
        exit(-1);
    }

    /* Creación del segmento al espacio de direcciones virtuales */
    vector = shmat(shmid, 0, 0);
    if (vector == (char *)-1) {
        perror("Error en la conexión");
        exit(-1);
    }

    /* Manipulación del segmento de memoria compartida */
    printf("Segmento mapeado (asociado)\n");
    ascii = 'a';
    for (i = 0; i < MAX; i++) {
        vector[i] = ascii;
        ascii = ascii+1;
    }

    for (i = 0; i < MAX; i++) {
        printf("Valor almacenado %c\n", vector[i]);
    }

    /* Desconexión del segmento al espacio de direcciones virtuales */
    if (shmdt(vector) == -1) {
        printf("Error en la desconexión");
        exit(-1);
    } else {
        printf("Éxito en la desconexión\n");
    }

    /* Borrado del segmento de memoria compartida */
    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        printf("Error en el borrado del segmento");
        exit(-1);
    } else {
        printf("Borrada la memoria compartida\n");
    }

    return 0;
}