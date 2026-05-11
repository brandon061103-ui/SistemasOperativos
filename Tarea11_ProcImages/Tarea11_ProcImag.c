//Un filtro de paso bajo:
//Elimina altas frecuencias
//En imágenes eso significa:
//   *Quita detalles finos
//   *Suaviza bordes
//   *Reduce ruido

//Habilita la implementación de la librería stb_image
//Permite usar funciones como stbi_load() para cargar imágenes
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//Habilita la implementación de la librería stb_image_write
//Permite guardar imágenes con funciones como stbi_write_jpg()
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <stdio.h> //Librería estándar de entrada y salida (printf)
#include <stdlib.h> //Librería estándar para manejo de memoria dinámica (malloc, free)
#include <omp.h> //Librería para paralelismo con OpenMP (uso de múltiples hilos)
//OpenMP (Open Multi-Processing) es una API que permite programación paralela en C usando hilos.

//Representación de la imagen: unsigned char *img, en un arreglo lineal (1D)
//Porque en memoria RAM todo es lineal.
//Fórmula: (py * width + px) * channels + c
//   py * width: Posiciona en la fila
//   + px: Mueve a la columna
//   * channels: Salta al bloque RGB
//   + c: Selecciona el canal (R, G o B)


//Para cada píxel:
//   Toma una ventana 3x3 alrededor
//   Suma todos los valores
//   Divide entre 9 (promedio)

//Reduce diferencias bruscas entre píxeles
//Elimina altas frecuencias (detalles finos)
//Genera una imagen más uniforme

//Función Paralela
//Aplica un filtro de paso bajo (blur) usando paralelismo con OpenMP
//y, x: Recorren la imagen
//c: Recorre canales (RGB)
//ky, kx: Recorren la ventana 3x3
void blur(unsigned char *input, unsigned char *output, int width, int height, int channels) {
    //Directiva de OpenMP:
    //Divide automáticamente el bucle entre 8 hilos (núcleos)
    #pragma omp parallel for num_threads(8)
    for (int y = 1; y < height - 1; y++) { //Recorre filas (evitando bordes)
        for (int x = 1; x < width - 1; x++) { //Recorre columnas

            for (int c = 0; c < channels; c++) { //Recorre cada canal (RGB)

                int sum = 0; //Acumulador para calcular el promedio

                //Recorre la ventana 3x3 (kernel)
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {

                        //Coordenadas del píxel vecino
                        int px = x + kx;
                        int py = y + ky;

                        //Acceso al arreglo 1D de la imagen:
                        //(fila * ancho + columna) * canales + canal actual
                        sum += input[(py * width + px) * channels + c];
                    }
                }

                //Promedio de los 9 valores (filtro de paso bajo)
                output[(y * width + x) * channels + c] = sum / 9;
            }
        }
    }
}


//Función secuencial
//Misma operación que blur(), pero sin paralelismo (un solo hilo)
void blur_secuencial(unsigned char *input, unsigned char *output, int width, int height, int channels) {
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            for (int c = 0; c < channels; c++) {
                int sum = 0;
                // Ventana 3x3
                for (int ky = -1; ky <= 1; ky++) {
                    for (int kx = -1; kx <= 1; kx++) {
                        int px = x + kx;
                        int py = y + ky;
                        sum += input[(py * width + px) * channels + c];
                    }
                }

                //Promedio de los píxeles vecinos
                output[(y * width + x) * channels + c] = sum / 9;
            }
        }
    }
}


//Función principal
int main(int argc, char *argv[]) {
    //Verifica que el usuario haya pasado el nombre de la imagen como argumento
    if (argc < 2) {
        printf("Uso: %s imagen.jpg\n", argv[0]);
        return 1;
    }

    int width, height, channels; //Variables para dimensiones de la imagen

    //Carga la imagen desde el archivo
    //argv[1] Nombre/ruta de la imagen
    //width, height, channels se llenan automáticamente
    unsigned char *img = stbi_load(argv[1], &width, &height, &channels, 0);

    //Verificación de error al cargar imagen
    if (!img) {
        printf("Error al cargar imagen\n");
        return 1;
    }

    //Reserva memoria para imágenes de salida (secuencial y paralelo)
    //No sabes tamaño de imagen en compilación y Permite manejar imágenes grandes
    unsigned char *out1 = malloc(width * height * channels);
    unsigned char *out2 = malloc(width * height * channels);

    //Medición secuencial
    double t1 = omp_get_wtime(); //Tiempo inicial
    blur_secuencial(img, out1, width, height, channels); //Procesamiento secuencial
    double t2 = omp_get_wtime(); //Tiempo final

    //Medición paralela
    double t3 = omp_get_wtime(); //Tiempo inicial
    blur(img, out2, width, height, channels); //Procesamiento paralelo
    double t4 = omp_get_wtime(); //Tiempo final

    //Impresión de resultados
    printf("Tiempo secuencial: %f\n", t2 - t1);
    printf("Tiempo paralelo: %f\n", t4 - t3);

    //Cálculo del speedup (aceleración)
    printf("Speedup: %f\n", (t2 - t1)/(t4 - t3));

    //Guardado de imágenes resultantes
    //Convierte el arreglo procesado a archivo JPG. Parámetro 100: Calidad máxima
    stbi_write_jpg("salida_secuencial.jpg", width, height, channels, out1, 100); //NombreFile, Ancho, Altura, Canales (RGB), TamMemoria, %Calidad
    stbi_write_jpg("salida_paralela.jpg", width, height, channels, out2, 100);

    //Liberación de memoria
    stbi_image_free(img);
    free(out1);
    free(out2);

    return 0;
}
//Distribuye automáticamente las iteraciones del bucle entre múltiples hilos.
//Se asignaron explícitamente 8 hilos para aprovechar los 8 núcleos del procesador.
