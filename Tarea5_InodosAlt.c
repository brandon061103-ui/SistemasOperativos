//Ejecuta el comando "ls -i <directorio>" usando popen() y muestra los inodos del directorio indicado por el usuario.
#include <stdio.h> // Entrada y salida estándar (printf, fprintf, fgets, FILE)
#include <stdlib.h> //Funciones de Manejo General (popen, pclose)

int main(int argc, char *argv[]) {
    //argc - Número de argumentos
    //argv - Arreglo de cadenas con los argumentos
    //Verifica que el usuario pasó exactamente un argumento, en este caso el directorio del que quiere conocer el número de inodos
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directorio>\n", argv[0]);
        return 1;
    }
    //Construcción del comando: ls -i <directorio>
    char command[512]; //Espacio reservados para el comando
    snprintf(command, sizeof(command), "ls -i %s", argv[1]); //El comando se guarda como texto

    //popen():
    // Ejecuta el comando en un proceso hijo para ejecutar ls -i
    // Devuelve un FILE* para leer su salida estándar
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Error al ejecutar ls");
        return 1;
    }

    //Lectura y muestra de la salida del comando
    char buffer[256]; //Espacio reservado para el buffer
    //Lee una línea de la salida del comando y la guarda en buffer
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer); //Imprimir los resultados del comando línea por línea
    }

    //Se cierra el proceso, espera a que termine el proceso hijo, libera recursos
    pclose(fp);

    return 0;
}
