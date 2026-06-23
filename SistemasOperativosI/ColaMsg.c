#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

/*Practica #12 - Cola de Mensajes*/
/*Objetivo: Aprender a crear, enviar, recibir y eliminar una cola de mensajes(Buzón)*/
/*Crear una cola de mensajes*/
int main (int argc, char *argv[]){
   int idcola;
   key_t clave;
   system("ipcs -q");

   /*Crear Cola*/
   clave = ftok(argv[0],'K');
   if((idcola = msgget(clave, IPC_CREAT|0666))<0){
      perror("msgget:create");
      exit(-1);
   }
   printf("Creada cola de mensaje con identificador = %d \n", idcola);
   /*Abre la cola de mensajes nuevamente*/
   if((idcola=msgget(clave,0))<0){
      perror("msgget:open");
      exit(-1);
   }
   printf("Abierta una cola de mesnsajes de identificador = %d\n", idcola);
   system("ipcs -q");
   exit(0);
}