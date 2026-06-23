#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>

#define TAM_BUFF 512

struct mensaje{
   long tipomsg;
   char msg[TAM_BUFF];
};

/*Elimina cola de mesnsajes creada previamente*/
int main (int argc, char *argv[]){
   int idcola;
   struct msqid_ds buffer;

   if(argc != 2){
      puts("MODO DE EMPLEO para Eliminar_cola se necesita de <identificador de cola>");
      exit(-1);
   }
   system("ipcs -q");
   idcola=atoi(argv[1]);
   if((msgctl(idcola, IPC_RMID,NULL))<0){
      perror("msgctl");
      exit(-1);
   }
   printf("Cola de mensajes %d eliminada\n", idcola);
   system("ipcs -a");
   exit(0);
}