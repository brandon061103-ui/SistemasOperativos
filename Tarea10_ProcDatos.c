#include <stdio.h> //Entrada y salida estándar (printf, fopen, fscanf)
#include <stdlib.h> //Funciones generales (malloc, free, exit)
#include <pthread.h> //Manejo de hilos (pthread_create, pthread_join)
#include <time.h> //Medición de tiempo (clock_gettime)
#include <limits.h> //Valores límite (INT_MAX, INT_MIN)

#define TOTAL_DATOS 10000000 //Cantidad total de datos a procesar
#define NUM_HILOS 4 //Número de hilos (simula 4 núcleos)
#define REPETICIONES 5 //Repeticiones para medir tiempo promedio
#define MAX_VALOR 1000 //Valor máximo posible en los datos

//Arreglo global donde se almacenan los datos leídos del archivo
int datos[TOTAL_DATOS];

//Variables globales compartidas entre hilos
long long suma_global; //Suma total de todos los datos
int minimo_global; //Valor mínimo encontrado
int maximo_global; //Valor máximo encontrado

//Arreglo para contar cuántas veces aparece cada número (frecuencia)
int frecuencias[MAX_VALOR + 1];

//Mutex para evitar que los hilos puedan acceder al mismo recurso de forma simultanea
pthread_mutex_t mutex;

//Estructura que define el rango de trabajo de cada hilo
typedef struct {
    int inicio; //Índice inicial del segmento
    int fin; //Índice final del segmento
} Rango;

//Función para la ejecución de los hilos
void *procesar_segmento(void *arg) {

    //Conversión del argumento genérico a estructura Rango
    Rango *r = (Rango *)arg;

    //Variables locales (cada hilo tiene sus propias variables)
    long long suma_local = 0;
    int minimo_local = INT_MAX;
    int maximo_local = INT_MIN;

    //Arreglo local para contar frecuencias
    int freq_local[MAX_VALOR + 1] = {0};

    //Recorrer el segmento asignado a este hilo
    for (int i = r->inicio; i < r->fin; i++) {
        int val = datos[i];

        suma_local += val; //Acumular suma

        //Encontrar mínimo local
        if (val < minimo_local)
            minimo_local = val;

        //Encontrar máximo local
        if (val > maximo_local)
            maximo_local = val;

        //Contar frecuencia del número
        freq_local[val]++;
    }

    //Sección crítica (solo un hilo entra a la vez)
    pthread_mutex_lock(&mutex);

    //Combinar resultados locales en variables globales
    suma_global += suma_local;

    if (minimo_local < minimo_global)
        minimo_global = minimo_local;

    if (maximo_local > maximo_global)
        maximo_global = maximo_local;

    //Sumar frecuencias locales al arreglo global
    for (int i = 1; i <= MAX_VALOR; i++)
        frecuencias[i] += freq_local[i];

    pthread_mutex_unlock(&mutex); //Liberar mutex

    return NULL; //Finaliza el hilo
}

//Función para calcular los tiempos de ejecución
double tiempo_diff(struct timespec a, struct timespec b) {
    //Calcula la diferencia entre dos tiempos en segundos
    return (b.tv_sec - a.tv_sec) + (b.tv_nsec - a.tv_nsec) / 1e9;
}

//Reinicio de las variables para tener el cálculo preciso del tiempo de ejecución
void reiniciar_variables() {
    suma_global = 0;
    minimo_global = INT_MAX;
    maximo_global = INT_MIN;

    //Reiniciar arreglo de frecuencias
    for (int i = 1; i <= MAX_VALOR; i++)
        frecuencias[i] = 0;
}

//Función para el cálculo de las frecuencias  (Qué número se repite en más y menos ocasiones)
void calcular_frecuencias(int *mas_frec, int *menos_frec) {
    int max_count = 0;
    int min_count = INT_MAX;

    for (int i = 1; i <= MAX_VALOR; i++) {

        //Buscar el número más frecuente
        if (frecuencias[i] > max_count) {
            max_count = frecuencias[i];
            *mas_frec = i;
        }

        //Buscar el número menos frecuente (existente)
        if (frecuencias[i] < min_count && frecuencias[i] > 0) {
            min_count = frecuencias[i];
            *menos_frec = i;
        }
    }
}

//Función principal
int main() {

    //Abrir archivo con los datos
    FILE *archivo = fopen("datos.txt", "r");

    if (!archivo) {
        perror("Error abriendo archivo");
        return 1;
    }

    //Leer los datos del archivo al arreglo
    for (int i = 0; i < TOTAL_DATOS; i++)
        fscanf(archivo, "%d", &datos[i]);

    //Cerrar el archivo
    fclose(archivo);

    //Inicializar mutex
    pthread_mutex_init(&mutex, NULL);

    //Realizar la prueba con 1 hilo
    struct timespec inicio1, fin1; //Vatiables para el cálculo del tiempo de ejecución
    double tiempo_1 = 0;
    //Realizar las operaciones diferentes veces para obtener un promedio de tiempo más preciso
    for (int r = 0; r < REPETICIONES; r++) {
        reiniciar_variables(); //Llamado a la función para reiniciar todas las variables
        pthread_t hilo; //Declaración del hilo único
        // Todo el trabajo lo hace un solo hilo
        Rango rango = {0, TOTAL_DATOS}; //Rango de datos que va a trabajar el hilo, el hilo procesará todos los datos
        clock_gettime(CLOCK_MONOTONIC, &inicio1); //Se obtiene el tiempo antes de iniciar la ejecución
        pthread_create(&hilo, NULL, procesar_segmento, &rango); //Se crea el hilo que ejecutará la función procesar_segmento
        pthread_join(hilo, NULL); //Se espera a que el hilo termine su ejecución
        clock_gettime(CLOCK_MONOTONIC, &fin1); //Se obtiene el tiempo al finalizar la ejecución
        tiempo_1 += tiempo_diff(inicio1, fin1); //Se acumula el tiempo que tardó esta ejecución
    }

    // Promedio del tiempo
    tiempo_1 /= REPETICIONES; //Se calcula el promedio del tiempo total
    double promedio_1 = (double)suma_global / TOTAL_DATOS; //Se calcula el promedio de los datos (suma total / cantidad de datos)
    int mas_frec_1, menos_frec_1; //Variables para almacenar el número más frecuente y el menos frecuente
    calcular_frecuencias(&mas_frec_1, &menos_frec_1); //Se calculan las frecuencias usando el arreglo global

    //Realizar la prueba con 4 hilos
    struct timespec inicio4, fin4; //Vatiables para el cálculo del tiempo de ejecución
    double tiempo_4 = 0;
    //Realizar las operaciones diferentes veces para obtener un promedio de tiempo más preciso
    for (int r = 0; r < REPETICIONES; r++) {
        reiniciar_variables(); //Llamado a la función para reiniciar todas las variables
        pthread_t hilos[NUM_HILOS]; //Arreglo de hilos (uno por cada núcleo que se desea usar)
        Rango rangos[NUM_HILOS]; //Arreglo de estructuras que define el rango de trabajo de cada hilo
        int bloque = TOTAL_DATOS / NUM_HILOS; //Tamaño del bloque que procesará cada hilo
        clock_gettime(CLOCK_MONOTONIC, &inicio4); //Se toma el tiempo antes de iniciar la ejecución paralela
        //Crear hilos y dividir trabajo
        for (int i = 0; i < NUM_HILOS; i++) {
            rangos[i].inicio = i * bloque; //Cálculo del índice inicial para cada hilo
            rangos[i].fin = (i == NUM_HILOS - 1) ? TOTAL_DATOS : (i + 1) * bloque; //El último hilo procesa hasta el final del arreglo
            pthread_create(&hilos[i], NULL, procesar_segmento, &rangos[i]); //Creación del hilo con su respectivo rango de trabajo
        }

        //Esperar a que todos los hilos terminen
        for (int i = 0; i < NUM_HILOS; i++)
            pthread_join(hilos[i], NULL);
        clock_gettime(CLOCK_MONOTONIC, &fin4); //Se toma el tiempo al finalizar todos los hilos
        tiempo_4 += tiempo_diff(inicio4, fin4); //Se acumula el tiempo de esta ejecución
    }
    tiempo_4 /= REPETICIONES; //Se calcula el tiempo promedio de ejecución paralela
    double promedio_4 = (double)suma_global / TOTAL_DATOS; //Se calcula el promedio de los datos
    int mas_frec_4, menos_frec_4; //Variables para almacenar resultados de frecuencia
    calcular_frecuencias(&mas_frec_4, &menos_frec_4); //Se calculan el número más y menos frecuente

    //Resultados después de realizar todas las operaciones
    double speedup = tiempo_1 / tiempo_4;
    printf("\nResultados con 1 hilo\n");
    printf("Tiempo: %.6f s\n", tiempo_1);
    printf("Promedio: %.2f\n", promedio_1);
    printf("Mas frecuente: %d\n", mas_frec_1);
    printf("Menos frecuente: %d\n", menos_frec_1);

    printf("\nResultados con 4 hilos\n");
    printf("Tiempo: %.6f s\n", tiempo_4);
    printf("Promedio: %.2f\n", promedio_4);
    printf("Mas frecuente: %d\n", mas_frec_4);
    printf("Menos frecuente: %d\n", menos_frec_4);

    printf("\nSpeedup observado:\n");
    printf("Speedup = %.2f\n", speedup);

    // Liberar recursos del mutex
    pthread_mutex_destroy(&mutex);
    return 0;
}
