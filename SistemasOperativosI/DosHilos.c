#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

//Hola mundo con hilos
void *hola(void *arg){
   char *msg="Hola";
   int i=0;
   for(i=0;i<strlen(msg);i++){
   printf("%c", msg[i]);
   fflush(stdout);
   usleep(1000000);
   printf("\n");
   }
   return NULL;
}

void *mundo(void *arg){
   char *msg="Mundo";
   int i=0;
   for(i=0;i<strlen(msg);i++){
   printf("%c", msg[i]);
   fflush(stdout);
   usleep(1000000);
   printf("\n");
   }
   return NULL;
}

int main(int argc, char *argv[]){
   pthread_t h1;
   pthread_t h2;
   pthread_create(&h1,NULL,hola,NULL);
   pthread_create(&h2,NULL,mundo,NULL);
   pthread_exit(&h1);
   pthread_exit(&h2);
   printf("Fin \n");
}