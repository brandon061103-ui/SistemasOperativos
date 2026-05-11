/* Lee un directorio indicado por línea de comandos y muestra
* Número de inodo
* Nombre del archivo
* Total de inodos encontrados.
Hace uso de opendir() y readdir(), que acceden directamente a la estructura interna del directorio en el sistema de archivos. */

/*
struct dirent {
    ino_t d_ino; // Número de inodo
    off_t d_off; // Offset (posición)
    unsigned short d_reclen; // Tamaño del registro
    unsigned char d_type; // Tipo de archivo (si existe)
    char d_name[]; // Nombre del archivo
};
*/

#include <stdio.h> //Entrada y salida estándar (printf(), perror())
#include <stdlib.h> //Finalizar el programa de forma controlada (exit(), EXIT_FAILURE)
#include <dirent.h> //Lectura del contenido de directorios (struct dirent, opendir(), readdir(), closedir)

int main(int argc, char *argv[]) {

/* Verificación de argumentos
* argc Número de argumentos recibidos.
* argv[0] Nombre del programa
* argv[1] Directorio a analizar
Debe recibir un argumento adicional. */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <directorio>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

/* Apertura del directorio
* opendir() Solicita al kernel abrir el directorio.
* Devuelve un puntero DIR* que representa el directorio
* Carga en memoria. */
    DIR *dir = opendir(argv[1]);
    // Si opendir falla, devuelve NULL
    if (dir == NULL) {
        perror("Error al abrir el directorio");
        exit(EXIT_FAILURE);
    }

/* Lectura de las entradas del directorio
- readdir() Devuelve un puntero a struct dirent, que representa una entrada del directorio.
Cada entrada contiene:
* d_ino - Número de inodo
* d_name - Nombre del archivo */
    struct dirent *entrada;
    int total_inodos = 0;

    printf("\nListado de inodos del directorio: %s\n\n", argv[1]);
    printf("%-15s %s\n", "Inodo", "Nombre");
    printf("---------------------------------------\n");

    // Leer todas las entradas hasta que readdir devuelva NULL
    while ((entrada = readdir(dir)) != NULL) {
/*
* entrada->d_ino:
- Número de inodo asociado al archivo.
- Se obtiene directamente del directorio, sin llamadas adicionales al kernel.

* entrada->d_name:
- Nombre del archivo dentro del directorio. */
        printf("%-15lu %s\n", (unsigned long) entrada->d_ino, entrada->d_name);
        total_inodos++;
    }

/* Cierre del directio
- closedir() Libera los recursos asociados al directorio. */
    closedir(dir);

/* Resultado
* Se imprime el total de inodos (entradas) encontrados. */
    printf("\nTotal de inodos en el directorio: %d\n", total_inodos);

    return 0;
}
