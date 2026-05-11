#include <stdio.h> //Entrada y salida estándar: printf(), fopen(), fclose(), fprintf() y perror()
#include <stdlib.h> //Funciones generales del sistema: EXIT_SUCCESS / EXIT_FAILURE y srand() y rand()
#include <time.h> //Librería para manejo de funciones relacionadas con el tiempo y fecha: time(NULL)

#define TOTAL_DATOS 10000000 //Cantidad total de números aleatorios que se generarán (10 millones)
#define MIN 1 // Valor mínimo que puede tomar cada número generado
#define MAX 1000 // Valor máximo que puede tomar cada número generado

int main()
{
    //Puntero a FILE para manejar el archivo de salida
    //fopen() abre (o crea) el archivo "datos.txt" en modo "w" (escritura)
    //Si el archivo ya existe, se sobrescribe completamente
    FILE *archivo = fopen("datos.txt", "w");

    //Verificación de errores al abrir/crear el archivo
    if (archivo == NULL) {
        perror("Error al crear el archivo");
        return 1; // Regresa 1 para indicar que el programa terminó con error
    }

    //Inicialización del generador de números pseudoaleatorios
    //time(NULL) devuelve el tiempo actual en segundos
    //srand() usa ese valor para que cada ejecución genere una secuencia diferente
    srand(time(NULL));

    //Bucle que genera exactamente 10.000.000 de números aleatorios
    for (int i = 0; i < TOTAL_DATOS; i++) {
        //Generación de un número aleatorio entre MIN y MAX
        //rand() genera un número entre 0 y RAND_MAX
        int numero = MIN + rand() % (MAX - MIN + 1);

        //Escritura del número en el archivo con un salto de línea
        fprintf(archivo, "%d\n", numero);
    }

    //Cierre del archivo
    fclose(archivo);

    //Notifica al usuario indicando que el proceso terminó correctamente
    printf("Se generaron %d datos en datos.txt\n", TOTAL_DATOS);

    return 0;
}
