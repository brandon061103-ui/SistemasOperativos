#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define SIZE 255

int main(int argc, char *argv[]){
   int mi_socket, tam, numbytes;
   char buffer[SIZE];
   struct sockaddr_in mi_estructura;
   if(argc != 3){
      printf("Error: modo de empleo: cliente ip puerto\n");
      exit(-1);
   }
   mi_estructura.sin_family = AF_INET;
   mi_estructura.sin_port = htons(atoi(argv[2]));
   inet_aton(argv[1], &(mi_estructura.sin_addr));
   memset( &(mi_estructura.sin_zero), '\0', 8);
   mi_socket = socket( AF_INET, SOCK_STREAM, 0 );
   tam = sizeof(struct sockaddr);
   connect( mi_socket, (struct sockaddr *)&mi_estructura, tam);
   numbytes = recv(mi_socket, buffer, SIZE-1, 0);
   buffer[numbytes] = '\0';
   printf("%id Bytes Recibidos\n", numbytes);
   printf("Recibido: %s\n", buffer);
   close(mi_socket);
}