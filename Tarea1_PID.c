#include <stdio.h> //Entrada y Salida
#include <stdlib.h> //Memoria Dinámica
#include <dirent.h> //Leer Directorios
#include <string.h> //Manipulación de Cadenas
#include <ctype.h> //Clasificar y Manipular Caracteres
#include <unistd.h> //Obtener el identificador del usuario
#include <sys/types.h> //Definir datos básicos del Sistema

//Definir un nodo de la lista ligada
typedef struct Node {
    int pid; //PID del proceso
    char *maps_content; //Puntero a una cadena con el contenido de maps
    struct Node *next; //Puntero al siguiente nodo de la lista
} Node;

//Creacion del Nodo
Node* create_node(int pid, const char *content) {
    Node *node = malloc(sizeof(Node)); //Asignación de memoria para un nuevo nodo (Memory ALLOCation)
    if (!node) return NULL;

    node->pid = pid; //Asigna PID al nodo
    node->maps_content = content ? strdup(content) : NULL; //Copia el contenido (String Duplicate) Asignando memoria nueva y copiando la cadena
    node->next = NULL; //Apuntador al final de la lista
    return node;
}

//Funcion de la Lista Ligada
void append_node(Node **head, Node *new_node) {
//Recibe un puntero al puntero de la cabeza de la lista para modificarlo si está vacía
    if (!*head) {
        *head = new_node; //Si la lista está vacía, asigna el nuevo nodo como la cabeza
        return;
    }
    Node *current = *head;
    while (current->next) current = current->next; //Si no recorre la lista hasta el último nodo
    current->next = new_node; //Asigna el nuevo nodo al "next" del final
}

//Funcion para abrir, leer y copiar el contenido de los archivos
char* read_maps_file(int pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid); //Construye la ruta del archivo con snprintf (String Print Formatted) para evitar overflows

    //Abre el archivo en modo lectura
    FILE *f = fopen(path, "r");
    if (!f) {
        return NULL;
    }

    // Leer todo el archivo
    char *buffer = NULL;
    size_t size = 0;
    char line[512];

    //Lectura linea por linea
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);
        buffer = realloc(buffer, size + len + 1); //Calcula la longitud de cada linea y redimensiona el buffer con REALLOC(ate)
        if (!buffer) {
            free(buffer);
            fclose(f);
            return NULL;
        }
        memcpy(buffer + size, line, len); //Copia la linea al final del buffer (Memory Copy)
        size += len; //Actualiza el tamaño
        buffer[size] = '\0'; //Asegura que termina en NULL
    }
    fclose(f); //Cierra el archivo
    return buffer ? buffer : strdup("( Archivo Vacío)\n");
}

// Imprimir el contenido de la lista ligada
// 1. Recorre la lista desde la cabeza
// 2. Para cada Nodo imprime su PID
// 3. Si hay contenido, lo imprime, de lo contrario imprime un mensaje de error
// 4. Imprime un separador
// 5. Avanza al siguiente nodo
void print_list(Node *head) {
    Node *current = head;
    while (current) {
        printf("PID: %d\n", current->pid);
        if (current->maps_content) {
            printf("%s", current->maps_content);
        } else {
            printf("(No se pudo leer)\n");
        }
        printf("------------------------\n");
        current = current->next;
    }
}

// Liberar los nodos despues de su uso
// 1, Recorre la lista
// 2. Guarda el nodo actual en temporal
// 3. Avanza la cabeza al siguiente
// 4. Libera el contenido y luego el nodo
// Importante para liberar toda la memoria dinámica y evitar leaks
void free_list(Node *head) {
    Node *tmp;
    while (head) {
        tmp = head;
        head = head->next;
        free(tmp->maps_content);
        free(tmp);
    }
}

//Programa Principal
int main() {
    //Checar el PID de uso y abrir la carpeta /proc
    printf("Ejecutando como usuario: %s (uid=%d)\n",
           getuid() == 0 ? "root" : "normal", getuid());

    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("No se pudo abrir /proc");
        return 1;
    }
    //Inicializa la lista vacía
    Node *head = NULL;
    struct dirent *entry;
    //Lee las entradas de solo directorios con readdir (Read Directory) y checa que sean números con isdigit
    while ((entry = readdir(dir))) {
        if (entry->d_type != DT_DIR) continue;

        char *name = entry->d_name;
        int is_number = 1;
        for (int i = 0; name[i]; i++) {
            if (!isdigit(name[i])) {
                is_number = 0;
                break;
            }
        }
        if (!is_number) continue;
        //Para cada PID válido lee el maps, crea el nodo, lo agrega a la lista, libera el contenido original.
        int pid = atoi(name);
        char *content = read_maps_file(pid);
        Node *node = create_node(pid, content);
        if (node) append_node(&head, node);
        free(content);   // ya se copió con strdup()
    }
    //Cerrar la direccion abierta
    closedir(dir);
    //Mostrar la lista ligada y liberar el espacio de la lista
    print_list(head);
    free_list(head);

    return 0;
}
