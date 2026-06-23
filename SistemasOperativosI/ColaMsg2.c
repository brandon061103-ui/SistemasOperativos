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

/*Enviar mensaje a la cola de mensajes creada previamente*/
int main (int argc, char *argv[]){
   int idcola;
   int tam;
   struct mensaje buffer;

   if(argc != 2){
      puts("MODO DE EMPLEO para Leer_cola se necesita de <identificador de cola>");
      exit(-1);
   }
   idcola=atoi(argv[1]);
   tam=msgrcv(idcola,&buffer,TAM_BUFF,0,0);
   if(tam>0){
      printf("Leyendo el idcola %05d\n",idcola);
      printf("\t Tipo de Mensaje: %05ld\n", (&buffer)->tipomsg);
      printf("\t Texto del Mensaje: %s\n", (&buffer)->msg);
   }else{
      perror("msgrcv");
      exit(-1);
   }
   exit(0);
}