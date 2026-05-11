#include <stdio.h> //Entrada y salida estándar: printf(), perror(), fopen(), fgets()
#include <stdlib.h> //Funciones generales: exit()
#include <string.h> //Manejo de cadenas: strstr(), strcmp(), strncmp(), strcpy()
#include <dirent.h> //Manejo de directorios: opendir(), readdir(), closedir()

#define MAX_TTY 200 //Número máximo de drivers TTY que se almacenarán
#define MAX_NAME 256 //Tamaño máximo para nombres de dispositivos

//Estructura para almacenar información de dispositivos
typedef struct {
    int major; //Número mayor (identifica el driver en el kernel)
    char name[MAX_NAME]; //Nombre del dispositivo (ej: tty, ttyS, pts)
} Device;

//Arreglo global para guardar dispositivos TTY encontrados
Device tty_devices[MAX_TTY];
int tty_count = 0;  //Contador de dispositivos encontrados

//Función para verificar si un dispositivo contiene "tty"
//(Teletipo) Interfaz que oermite la comunicación entre el ususario y el sistema operativo.
//Si encuentra "tty" devuelve un puntero (verdadero)
//Si no encuentra devuelve NULL (falso)
int is_tty_device(char *line) {
    return (strstr(line, "tty") != NULL); //strstr (String Matching) - Busca una subcadena dentro de otra cadena
}

//Función para leer /proc/devices
//Aquí accedemos a información del kernel sobre drivers registrados
void read_proc_devices() {
    FILE *file = fopen("/proc/devices", "r"); //Abrir archivo en modo lectura
    if (!file) {
        perror("Error al abrir /proc/devices");
        exit(1);
    }

    char line[256]; //Buffer para almacenar cada línea leída
    int in_char_section = 0; //Bandera para detectar sección de Dispositivos de Caracteres (Character devices)

    //Leer archivo línea por línea
    while (fgets(line, sizeof(line), file)) {
        // Detectar sección de dispositivos de caracteres (character devices)
        if (strstr(line, "Character devices:")) { //Activa el modo lectura de dispositivos tipo carácter
            in_char_section = 1;
            continue;
        }
        //Deja de leer porque llega al dispositivos de bloque (block devices), no importa esta sección
        if (strstr(line, "Block devices:")) {
            break;
        }

        // Procesar solo si estamos en la sección correcta
        if (in_char_section) {
            int major; //Numero mayor
            char name[MAX_NAME]; //Nombre del dispositivo
            //Extraer los datos del número mayor y el nombre del dispositivo
            if (sscanf(line, "%d %s", &major, name) == 2) {
                // Filtrar solo dispositivos TTY para almacenarse
                if (is_tty_device(name)) {
                    // Evitar overflow del arreglo
                    if (tty_count < MAX_TTY) {
                        tty_devices[tty_count].major = major; ////Guarda el número mayor
                        strcpy(tty_devices[tty_count].name, name); //Guarda el nombre del dispositivo
                        tty_count++; //Aumenta el contador de dispositivos encontrados
                    }
                }
            }
        }
    }

    fclose(file); //Cerrar el archivo
}

// Función para clasificar tipo de TTY según su nombre
const char* classify_tty(const char *name) {
    if (strncmp(name, "ttyS", 4) == 0) //ttyS - Puerto serial
        return "Puerto Serial";
    else if (strncmp(name, "tty", 3) == 0 && strlen(name) > 3) //tty1, tty2 - Terminal virtual
        return "Terminal Virtual";
    else if (strncmp(name, "pts", 3) == 0) //pts - Terminales usadas por SSH (Secure Shell) o GUI (Interfaz gráfica de usuario)
        return "Pseudo-Terminal";
    else if (strcmp(name, "tty") == 0) //dev/tty - Terminal actual
        return "Terminal TTY Actual";
    else if (strcmp(name, "ptmx") == 0) //Controlador de pseudo-terminales
        return "PTY Master";
    else if (strncmp(name, "hvc", 3) == 0) //Usado en virtualización (WSL, VM)
        return "Hypervisor Console";
    else
        return "Otro";
}

//Función para obtener major y minor desde sysfs
//Lee el archivo: /sys/class/tty/<dispositivo>/dev
//Este archivo contiene: major:minor
void get_dev_numbers(const char *tty_name, int *major, int *minor) {
    //Ruta en la que se buscara el número mayor y menor de los dispositivos
    char path[256];

    //Construir ruta al archivo dev
    sprintf(path, "/sys/class/tty/%s/dev", tty_name);

    FILE *file = fopen(path, "r");

    //Si no se puede abrir (algunos dispositivos no lo tienen)
    if (!file) {
        *major = -1;
        *minor = -1;
        return;
    }

    //Leer formato "major:minor"
    fscanf(file, "%d:%d", major, minor);
    //Cerrar el archivo
    fclose(file);
}

//Función para leer /sys/class/tty
//Aquí accedemos a dispositivos reales expuestos por el kernel (sysfs)
void list_sys_tty() {
    DIR *dir = opendir("/sys/class/tty"); //Abrir el directorio
    if (!dir) {
        perror("Error al abrir /sys/class/tty"); //Si falla mandar error y termina
        exit(1);
    }
    //Iterar sobre archivos
    struct dirent *entry;

    printf("\nDispositivos TTY encontrados:\n");

    //Recorrer cada dispositivo en el directorio
    while ((entry = readdir(dir)) != NULL) {

        //Ignorar "." y ".." (Directorio Actual y Directorio Padre)
        if (entry->d_name[0] == '.')
            continue;
        //Variables para almacenar el número mayor y menor
        int major, minor;

        //Obtener major y minor para cada dispositivo
        get_dev_numbers(entry->d_name, &major, &minor);

        //Mostrar nombre del dispositivo, tipo de clasificación número mayor y número menor.
        printf("%s -> %s -> Major: %d Minor: %d\n", entry->d_name, classify_tty(entry->d_name), major, minor);
    }

    closedir(dir); //Cerrar directorio
}

//Función principal
int main() {
    //Leer /proc/devices
    printf("Lectura de /proc/devices\n");
    //Leer drivers registrados en el kernel
    read_proc_devices();
    //Imprimir los resultados encontrados
    printf("Drivers TTY encontrados:\n");

    // Mostrar drivers encontrados (major numbers)
    for (int i = 0; i < tty_count; i++) {
        printf("Major: %d -> %s\n", tty_devices[i].major, tty_devices[i].name);
    }

    // Mostrar dispositivos reales y sus atributos
    list_sys_tty();

    return 0;
}
//1. Lee el kernel /proc/devices - Conocer los drivers registrados
//2. Filtra TTY - Identifica dispositivos de terminal
//3. Explora el sistema /sys/class/tty - Conocer los dispositivos reales
//4. Clasifica dispositivos (virtuales, seriales, pseudo-terminales)
//5. Extraer el número mayor y menor
