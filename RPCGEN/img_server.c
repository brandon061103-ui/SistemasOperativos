#include "imgproc.h" /*Archivo generado por rpcgen,: Estructuras (image, result), Prototipos de funciones RPC, Definiciones del programa*/
#include <stdlib.h> /*Librería estándar para: malloc() (memoria dinámica)*/
#include <omp.h> /*Librería para paralelismo con OpenMP (uso de múltiples hilos), Medir tiempos (omp_get_wtime()*/

/* Blur paralelo, misma función usada anteriormente */
void blur(unsigned char *input, unsigned char *output, int width, int height, int channels) {
    #pragma omp parallel for num_threads(8)
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < channels; c++) {

                int sum = 0;

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {

                        int px = x + kx;
                        int py = y + ky;

                        sum += input[(py * width + px) * channels + c];
                    }
                }

                output[(y * width + x) * channels + c] = sum / 9;
            }
        }
    }
}

/* Blur secuencial */
void blur_secuencial(unsigned char *input, unsigned char *output, int width, int height, int channels) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < channels; c++) {

                int sum = 0;

                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {

                        int px = x + kx;
                        int py = y + ky;

                        sum += input[(py * width + px) * channels + c];
                    }
                }

                output[(y * width + x) * channels + c] = sum / 9;
            }
        }
    }
}

/* Función RPC
result *: Retorna un puntero a estructura
Procesar: Nombre definido en .x
_1: Versión 1
_svc: Función del servidor

image *img
Contiene:
 *Ancho (width)
 *Alto (height)
 *Canales (channels)
 *Datos de imagen (data_val)

struct svc_req *req
Información de la petición:
 *IP del cliente
 *protocolo (TCP/UDP)
No se usa, pero RPC lo requiere
*/
result * procesar_1_svc(image *img, struct svc_req *req) {

    /*Variable estática RPC devuelve punteros. Si no es static: La memoria se pierde al salir de la función*/
    static result res;
    /*Calcula tamaño total de la imagen en bytes
    Pixeles = Ancho × Alto
    Bytes = Pixeles × Canales*/
    int size = img->width * img->height * img->channels;

    /*Reserva memoria para:
    out1: Imagen procesada secuencial
    out2: Imagen procesada paralela
    malloc asigna memoria en heap
    No se libera aquí porque RPC la usará después*/
    unsigned char *out1 = malloc(size);
    unsigned char *out2 = malloc(size);

    /*Usar data_val
    Se convierte porque:
     *STB trabaja con unsigned char
     *RPC usa char*/
    unsigned char *input = (unsigned char *) img->data.data_val;

    /* Tiempo secuencial */
    double t1 = omp_get_wtime();
    blur_secuencial(input, out1, img->width, img->height, img->channels);
    double t2 = omp_get_wtime();

    /* Tiempo paralelo */
    double t3 = omp_get_wtime();
    blur(input, out2, img->width, img->height, img->channels);
    double t4 = omp_get_wtime();
    /*Guarda tiempos en la estructura de salida*/
    res.time_seq = t2 - t1;
    res.time_par = t4 - t3;
    /*Prepara respuesta RPC*/
    /* Resultado secuencial */
    /*Copia metadatos de la imagen original*/
    res.img_seq.width = img->width;
    res.img_seq.height = img->height;
    res.img_seq.channels = img->channels;
    /*Guarda la imagen procesada (secuencial)
    Casting:
     *out1 es unsigned char *
     *RPC requiere char **/
    res.img_seq.data.data_val = (char *) out1;
    /*Indica tamaño del buffer*/
    res.img_seq.data.data_len = size;

    /* Resultado paralelo */
    res.img_par.width = img->width;
    res.img_par.height = img->height;
    res.img_par.channels = img->channels;
    res.img_par.data.data_val = (char *) out2;
    res.img_par.data.data_len = size;
    /*Retorna dirección de la estructura*/
    return &res;
}
