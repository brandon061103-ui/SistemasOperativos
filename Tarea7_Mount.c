#include <stdio.h> //Entrada y salida estándar, printf ().
#include <sys/mount.h> //Llamados al sistema, permite usar funciones y constantes para montar archivos, mount(), MS_BIND (mount flag)
#include <sys/stat.h> //Llamados al sistema, permite trabajar con atributos de archivos y directorios, mkdir()
#include <errno.h> //Indica por qué falló una llamada al sistema, errno () , EEXIST (Error: el archivo ya existe)

//Definción de rutas
#define SOURCE "/root/SistemasOperativosII/Tareas" //Carpeta donde están todos los archivos de trabajo.
#define TARGET "/mnt/Respaldo" //Carpeta objetivo donde se montará la carpeta.

int main() {
    //Creación de la carpeta de destino
    if (mkdir(TARGET, 0755) == -1 && errno != EEXIST) { //Asignación de ruta del directorio y permisos.
        perror("Error al crear el directorio destino"); //Comprobación de erorres, mkdir() devuelve -1 si hay error.
        return 1; //Termina el programa indicando que hubo error.
    }

    //Montar la carpeta
    //mount(origen, destino, tipo_fs, banderas, datos)
    //Origen: /root/SistemasOperativosII/Tareas, Destino: /mnt/Respaldo, Tipo de sistema de archivos: Ninguno, Banderas: Indica montaje bind, Datos: Ninguno.
    if (mount(SOURCE, TARGET, NULL, MS_BIND, NULL) == -1) { //MS_BIND -> Crea una segunda referencia al mismo árbol de archivos
        perror("Error al montar");
        return 1; //Termina el programa indicando que hubo error.
    }

    printf("Montaje realizado:\n");
    printf("%s montado en %s\n", SOURCE, TARGET);

    printf("\nPara desmontar usa:\n");
    printf("umount %s\n", TARGET);

    return 0;
}
