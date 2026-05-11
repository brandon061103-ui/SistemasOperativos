#include <stdio.h> //Entrada y salida estándar: printf()
#include <stdlib.h> ////Funciones generales: exit()
#include <string.h> //Manejo de cadenas: strtok(), strcmp(), strlen(), sscanf()
#include <fcntl.h> //Control de archivos: open()
#include <unistd.h> //Interfaz POSIX (comunicación con kernel): read(), write(), close()
#include <dirent.h> //Manejo de directorios: opendir() readdir() closedir()
#include <sys/ioctl.h> //Control directo de dispositivos, ioctl(), Comunicación con drivers ioctl
#include <linux/fs.h> //Constantes del kernel BLKGETSIZE64 (obtener tamaño del disco)

//Tamaño del buffer para lectura
#define BUFFER_SIZE 2048

//Función para leer atributos desde uevent usando syscalls
void read_uevent(const char *device) {
    //Variable para almacenar la ruta
    char path[256];

    //Construir ruta al archivo uevent
    sprintf(path, "/sys/class/block/%s/uevent", device);

    //open(): syscall para abrir archivo
    int fd = open(path, O_RDONLY); //Abre archivo en modo lectura, Devuelve un file descriptor (fd)
    if (fd < 0) {
        return; // Algunos dispositivos no tienen uevent accesible
    }
    //Variable para almacena el contenido del buffer
    char buffer[BUFFER_SIZE];

    //read(): Leer contenido del archivo
    int bytes = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes < 0) {
        close(fd);
        return;
    }

    buffer[bytes] = '\0'; //Convierte buffer en string válido

    //Mostrar encabezado
    printf("\nDispositivo: %s\n", device);

    //Separar líneas (KEY=VALUE)
    char *line = strtok(buffer, "\n");

    //Recorre cada línea del archivo
    while (line != NULL) {

        char key[128], value[128];

        //Extraer clave y valor
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {

            //write():Imprimir directamente en stdout
            write(1, key, strlen(key)); //Imprimir la clave
            write(1, ": ", 2);
            write(1, value, strlen(value)); //Imprimir el valor
            write(1, "\n", 1);
        }
        //Continúa con la siguiente línea
        line = strtok(NULL, "\n");
    }
    //Cierra el descriptor
    close(fd);
}

//Función para obtener tamaño del dispositivo usando ioctl
void get_device_size(const char *device) {

    char path[256];

    //Ruta al dispositivo en /dev
    sprintf(path, "/dev/%s", device);

    //Abrir dispositivo
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        printf("No se pudo abrir %s para ioctl\n", path);
        return;
    }

    //Variable para almacenar el tamaño del dispositivo
    unsigned long long size;

    //ioctl(): Obtener tamaño del dispositivo en bytes
    if (ioctl(fd, BLKGETSIZE64, &size) == 0) {
        printf("Tamaño del dispositivo: %llu bytes\n", size);
    } else {
        printf("No se pudo obtener tamaño con ioctl\n");
    }

    close(fd);
}

//Función para recorrer todos los dispositivos en /sys/class/block
void list_block_devices() {
    //Abrir el directorioo de /sys/class/block
    DIR *dir = opendir("/sys/class/block");

    if (!dir) {
        perror("Error al abrir /sys/class/block");
        exit(1);
    }
    //Iterar sobre archivos
    struct dirent *entry;

    printf("Dispositivos de bloque encontrados:\n");

    //Recorrer cada dispositivo en el directorio
    while ((entry = readdir(dir)) != NULL) {

        //Ignorar "." y ".." (Directorio Actual y Directorio Padre)
        if (entry->d_name[0] == '.')
            continue;

        //Leer atributos del dispositivo
        read_uevent(entry->d_name);

        //Obtener tamaño usando ioctl
        get_device_size(entry->d_name);
    }
    //Cerrar directorio
    closedir(dir);
}

//Función principal
int main() {

    printf("Exploración de dispositivos desde el kernel\n");

    // Recorrer todos los dispositivos de bloque
    list_block_devices();

    return 0;
}

//1. Recorre los dispositivos en /sys/class/block
//2. Lee atributos del kernel desde uevent
//3. Usa llamadas al sistema (syscalls): open(), read(), write(), ioctl()
//4. Obtiene información real del dispositivo (incluyendo tamaño)
