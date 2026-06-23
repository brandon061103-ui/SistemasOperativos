#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>

#define ARRAY_SIZE   4000
#define MALLOC_SIZE 10000
#define SHM_SIZE    10000
#define SHM_MODE    (SHM_R | SHM_W)     /* read/write */

char array[ARRAY_SIZE];  /* datos sin inicializar = bss */

int main() {
   int shmid;
   char *ptr, *shmptr;

   printf("array[] desde %x hasta %x\n", &array[0], &array[ARRAY_SIZE]);
   printf("stack sobre %x\n" ,&shmid);

   if ((ptr= malloc(MALLOC_SIZE)) == NULL)
      fprintf(stderr, "error de malloc()\n");
   printf("malloc desde %x hasta %x\n", ptr, ptr+MALLOC_SIZE);

   if ((shmid=shmget(IPC_PRIVATE, SHM_SIZE, SHM_MODE))<0)
      fprintf(stderr, "error de shmget()\n");
   if ((shmptr=shmat(shmid, 0, 0)) == (void *) -1)
      fprintf(stderr, "error de shmat()\n");
   printf("shared memory desde %x hasta %x\n", shmptr, shmptr+SHM_SIZE);
   /* proceso padre */
   if (0!=fork()) {
      while (*shmptr != 'x') ;
      printf("he comprobado 'x' en memoria compartida\npulse una tecla");
      getchar(); /* comprobar el semaforo con 'ipcs' */
      /* eliminar memoria compartida */
      if (shmctl(shmid, IPC_RMID, 0) < 0)
         fprintf(stderr,"error de shmctl()\n");

      exit(0);
   /* proceso hijo */
      }
   else {
      *shmptr ='x';
      exit(0);
        }
}