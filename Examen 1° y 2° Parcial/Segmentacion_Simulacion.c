#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

// Tamaño simulado de memoria física (3 GB)
#define MEM_REAL (3UL * 1024UL * 1024UL * 1024UL)
//#define MEM_REAL (128 * 1024UL)

// Tamaño máximo de una línea de /proc/PID/maps
#define TAM_LINEA 512

//Representa un segmento de memoria virtual obtenido del archivo /proc/PID/maps
typedef struct Segmento {
    pid_t pid; // Proceso dueño
    unsigned long v_inicio; // Dirección virtual inicial
    unsigned long v_fin; // Dirección virtual final
    size_t tam; // Tamaño del segmento
    void *dir_f; // Dirección en memoria física
    bool cargado; // ¿Está cargado en memoria física?
    struct Segmento *sig;
} Segmento;

//Representa un espacio libre en memoria física
typedef struct Mem_Libre {
    void *inicio; // Dirección inicial del hueco
    size_t tam; // Tamaño del hueco
    struct Mem_Libre *sig;
} Mem_Libre;


//Variables globales
void *Memoria_Fisica = NULL; // Memoria física simulada
Mem_Libre *Lista_Mem_Libre = NULL; // Lista de segmentos de M.R libres
Segmento *Lista_Segmentos = NULL; // Lista de segmentos de todos los procesos

// Inicialización de memoria física
//Reserva la memoria física simulada e inicializa un único hueco libre
void inicializar_memoria_fisica() {
    Memoria_Fisica = malloc(MEM_REAL);
    if (!Memoria_Fisica) {
        perror("Error al reservar memoria física");
        exit(1);
    }

    Lista_Mem_Libre = malloc(sizeof(Mem_Libre));
    Lista_Mem_Libre->inicio = Memoria_Fisica;
    Lista_Mem_Libre->tam = MEM_REAL;
    Lista_Mem_Libre->sig = NULL;

    printf("[INIT] Memoria física simulada: %lu MB\n",
           MEM_REAL / (1024 * 1024));
}

//Gestión de segmentos libres en M.R (FIRST-FIT)
//Busca el primer hueco donde quepa el segmento (Algoritmo First-Fit)
Mem_Libre* buscar_hueco_first_fit(size_t tam) {
    Mem_Libre *actual = Lista_Mem_Libre;

    while (actual) {
        if (actual->tam >= tam)
            return actual;
        actual = actual->sig;
    }
    return NULL;
}

//Asigna un segmento dentro de un hueco libre
//El segmento virtual ahora tiene una traducción física
void asignar_segmento(Mem_Libre *hueco, Segmento *seg) {
    seg->dir_f = hueco->inicio; //Equivale a escribir el campo BASE de la tabla de segmentos
    seg->cargado = true;

    hueco->inicio = (char*)hueco->inicio + seg->tam; //[ 100 KB libre ] -> [40 KB ocupados][60 KB libre]
    hueco->tam -= seg->tam;
}

//Libera un segmento y genera un nuevo hueco libre
void liberar_segmento(Segmento *seg) {
    Mem_Libre *nuevo = malloc(sizeof(Mem_Libre));
    nuevo->inicio = seg->dir_f; //Dirección Física que Ocupaba el Segmento
    nuevo->tam = seg->tam;
    nuevo->sig = Lista_Mem_Libre;

    Lista_Mem_Libre = nuevo;

    seg->dir_f = NULL;
    seg->cargado = false;
}

//Lee el archivo /proc/PID/maps completo
char* leer_maps(pid_t pid) {
    char ruta[64];
    snprintf(ruta, sizeof(ruta), "/proc/%d/maps", pid);

    FILE *f = fopen(ruta, "r");
    if (!f) return NULL;

    char *buffer = NULL;
    size_t tam = 0;
    char linea[TAM_LINEA];

    while (fgets(linea, sizeof(linea), f)) {
        size_t len = strlen(linea);
        buffer = realloc(buffer, tam + len + 1);
        memcpy(buffer + tam, linea, len);
        tam += len;
        buffer[tam] = '\0';
    }

    fclose(f);
    return buffer;
}

//Convierte el contenido de maps en segmentos
void crear_segmentos(pid_t pid, const char *maps) {
    unsigned long inicio, fin;

    while (*maps) {
        if (sscanf(maps, "%lx-%lx", &inicio, &fin) == 2) {
            Segmento *seg = malloc(sizeof(Segmento));
            seg->pid = pid;
            seg->v_inicio = inicio;
            seg->v_fin = fin;
            seg->tam = fin - inicio;
            seg->dir_f = NULL;
            seg->cargado = false;

            seg->sig = Lista_Segmentos;
            Lista_Segmentos = seg;
        }

        maps = strchr(maps, '\n');
        if (!maps) break;
        maps++;
    }
}

//Simulación de segmentos
//1. Toma un segmento virtual
//2. Busca hueco
//3. Asigna memoria física
//4. Espera un tiempo
//5. Libera memoria
void simular_segmentacion() {
    Segmento *seg = Lista_Segmentos;

    while (seg) {
        printf("\n[PID %d] Segmento %lx-%lx (%zu KB)\n",
               seg->pid,
               seg->v_inicio,
               seg->v_fin,
               seg->tam / 1024);

        Mem_Libre *hueco = buscar_hueco_first_fit(seg->tam);

        if (!hueco) {
            printf("No hay espacio en memoria física\n");
            seg = seg->sig;
            continue;
        }

        asignar_segmento(hueco, seg);
        int tiempo = rand() % 3000 + 500;
        //Imprime una dirección de memoria, muestra el valor del puntero
        //seg->dir_f = hueco->inicio y Memoria_Fisica + desplazamiento
        //Una dirección virtual del proceso que representa una dirección física simulada.
        printf("Asignado en %p | Tiempo %d ms\n",
               seg->dir_f, tiempo);

        usleep(tiempo * 1000);

        liberar_segmento(seg);
        printf("Segmento liberado\n");

        seg = seg->sig;
    }
}

int main() {
    srand(time(NULL));

    printf("=== Simulación de segmentación ===\n");

    inicializar_memoria_fisica();

    DIR *dir = opendir("/proc");
    struct dirent *entrada;

    while ((entrada = readdir(dir)) != NULL) {
        if (!isdigit(entrada->d_name[0]))
            continue;

        pid_t pid = atoi(entrada->d_name);
        char *maps = leer_maps(pid);

        if (maps) {
            crear_segmentos(pid, maps);
            free(maps);
        }
    }

    closedir(dir);

    simular_segmentacion();

    printf("\n=== Fin de la simulación ===\n");
    return 0;
}
