/*Habilita la implementación de la librería stb_image
Permite usar funciones como stbi_load() para cargar imágenes*/
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

/*Habilita la implementación de la librería stb_image_write
Permite guardar imágenes con funciones como stbi_write_jpg()*/
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/*Archivo generado por rpcgen
Contiene:Estructuras (image, result), Funciones remotas (procesar_1), Constantes (IMG_PROG, IMG_VERS)*/
#include "imgproc.h"
#include <stdio.h> /*Librería estándar de entrada y salida (printf)*/

/*Función principal*/
int main(int argc, char *argv[]) {
    /*Verifica que el usuario haya pasado el nombre de la imagen como argumento*/
    if (argc < 2) {
        printf("Uso: %s imagen.jpg\n", argv[0]);
        return 1;
    }
    /*Variables para dimensiones de la imagen (Width, Height, Channels)*/
    int w, h, c;

    /*Carga la imagen desde disco
    Parámetros:
     *argv[1]: Nombre del archivo
     *&w, &h, &c: Se llenan automáticamente. Dirección de memoria de esta variable
     *0 → mantiene formato original
    Retorna: Puntero a los datos de la imagen*/
    unsigned char *img_data = stbi_load(argv[1], &w, &h, &c, 0);

    /*Verifica que la imagen se cargó correctamente*/
    if (!img_data) {
        printf("Error al cargar imagen\n");
        return 1;
    }
    /*Crea la conexión para RPC
    Parámetros:
     *"localhost": Servidor (misma máquina)
     *IMG_PROG: Identificador del programa RPC
     *IMG_VERS: Versión
     *"tcp": Protocolo de comunicación
    Se usa TCP porque:
     *Soporta grandes cantidades de datos (imágenes)
     *UDP tiene límite (~64KB)*/
    CLIENT *cl = clnt_create("localhost", IMG_PROG, IMG_VERS, "tcp");

    /*Verifica si la conexión falló: clnt_pcreateerror() Muestra error detallado*/
    if (!cl) {
        clnt_pcreateerror("Error conexión");
        return 1;
    }

    /*Crea estructura definida en .x: Representa la imagen que se enviará al servidor*/
    image img;
    /*Llena metadatos de la imagen*/
    img.width = w;
    img.height = h;
    img.channels = c;
    /*img_data: Unsigned char *
     RPC usa: char *
    Se hace casting para compatibilidad
    Casting: Convertir un valor de un tipo de dato a otro*/
    img.data.data_val = (char *) img_data;
    /*Tamaño total de la imagen en bytes*/
    img.data.data_len = w * h * c;
    /*Llamada remota*/
    /*1.Se serializa la estructura (img)
      2.Se envía al servidor
      3.El servidor ejecuta procesar_1_svc
      4.Devuelve un result*/
    result *res = procesar_1(&img, cl);
    /*Verifica si la llamada falló*/
    if (!res) {
        clnt_perror(cl, "Error en RPC");
        return 1;
    }
    /*Muestra tiempos devueltos por el servidor. speedup = tiempo_secuencial / tiempo_paralelo*/
    printf("Tiempo secuencial: %f\n", res->time_seq);
    printf("Tiempo paralelo: %f\n", res->time_par);
    printf("Speedup: %f\n", res->time_seq / res->time_par);

    /*Guardado de imágenes resultantes
    Convierte el arreglo procesado a archivo JPG.*/
    stbi_write_jpg("rpc_seq.jpg", w, h, c, res->img_seq.data.data_val, 100); /*NombreFile, Ancho, Altura, Canales (RGB), Datos RPC, %Calidad*/
    stbi_write_jpg("rpc_par.jpg", w, h, c, res->img_par.data.data_val, 100);
    /*Cierra conexión RPC*/
    clnt_destroy(cl);
    /*Libera memoria de la imagen original*/
    stbi_image_free(img_data);

    return 0;
}
