#include <stdio.h> // Entrada y salida estándar (printf, perror)
#include <stdlib.h> // Memoria dinámica (malloc, free, realloc)
#include <dirent.h> // Manejo de directorios (/proc)
#include <string.h> // Manejo de cadenas (strlen, strdup, memcpy, strcmp)
#include <ctype.h> // Clasificación de caracteres (isdigit)
#include <unistd.h> // Obtener y hacer uso de fork, getpid, getuid
#include <sys/types.h> // Tipos del sistema (pid_t)
#include <signal.h> // Manejo de señales (kill, SIGTERM)

//Estructura de un nodo de la lista
typedef struct Node {
    pid_t pid; // PID del proceso
    char *maps_content; // Contenido de /proc/PID/maps
    struct Node *next; // Siguiente nodo de la lista principal
    struct Node *child; // Apunta al nodo del proceso hijo
} Node;

//Crea un nuevo nodo para un proceso
Node* create_node(pid_t pid, const char *content) {
    Node *node = malloc(sizeof(Node)); // Reserva memoria para el nodo
    if (!node) return NULL;

    node->pid = pid;
    // Duplica el contenido para que el nodo sea dueño de su memoria
    node->maps_content = content ? strdup(content) : NULL;
    node->next = NULL; // No apunta a nadie inicialmente
    node->child = NULL; // No tiene hijo inicialmente

    return node;
}

//Agrega un nodo al final de la lista ligada principal
void append_node(Node **head, Node *new_node) {
    // Si la lista está vacía, el nuevo nodo es la cabeza
    if (!*head) {
        *head = new_node;
        return;
    }

    // Recorre hasta el último nodo
    Node *current = *head;
    while (current->next) {
        current = current->next;
    }

    // Enlaza el nuevo nodo al final
    current->next = new_node;
}

//Lee el archivo /proc/PID/maps completo y lo retorna en memoria dinámica
char* read_maps_file(pid_t pid) {
    char path[64];
    snprintf(path, sizeof(path), "/proc/%d/maps", pid);

    FILE *f = fopen(path, "r");
    if (!f) return NULL;

    char *buffer = NULL; // Buffer dinámico
    size_t size = 0; // Tamaño actual del buffer
    char line[512];

    // Lee el archivo línea por línea
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);

        // Redimensiona el buffer para agregar la nueva línea
        char *tmp = realloc(buffer, size + len + 1);
        if (!tmp) {
            free(buffer);
            fclose(f);
            return NULL;
        }
        buffer = tmp;

        // Copia la línea al final del buffer
        memcpy(buffer + size, line, len);
        size += len;
        buffer[size] = '\0'; // Terminador nulo
    }

    fclose(f);

    // Si el archivo estaba vacío
    return buffer ? buffer : strdup("(Archivo vacío)\n");
}

//Imprime la lista de procesos, si un nodo tiene hijo, lo imprime como tal
void print_list(Node *head) {
    Node *current = head;

    while (current) {
        printf("PID: %d\n", current->pid);

        if (current->maps_content) {
            printf("%s", current->maps_content);
        } else {
            printf("(No se pudo leer maps)\n");
        }

        // Si este nodo representa al padre y tiene un hijo
        if (current->child) {
            printf("Proceso hijo:\n");
            printf("PID: %d\n", current->child->pid);

            if (current->child->maps_content) {
                printf("%s", current->child->maps_content);
            } else {
                printf("(No se pudo leer maps del hijo)\n");
            }
        }

        printf("-----------------------------\n");
        current = current->next;
    }
}

// Libera toda la memoria dinámica usada por la lista, incluye los nodos hijos
void free_list(Node *head) {
    Node *current = head;

    while (current) {
        Node *next = current->next;

        // Libera el nodo hijo si existe
        if (current->child) {
            free(current->child->maps_content);
            free(current->child);
        }

        free(current->maps_content);
        free(current);

        current = next;
    }
}

int main() {
    // PID del proceso padre
    pid_t parent_pid = getpid();
    printf("Proceso padre con PID: %d\n", parent_pid);

    // Creación del proceso hijo
    pid_t child_pid = fork();
    if (child_pid < 0) {
        perror("Error en fork");
        return 1;
    }

    if (child_pid == 0) {
        // Código del proceso hijo
        printf("Proceso hijo con PID: %d\n", getpid());
        while (1); // Mantener vivo el hijo
    }

    Node *head = NULL; // Cabeza de la lista principal
    Node *parent_node = NULL; // Nodo que representa al proceso padre
    Node *child_node = NULL; // Nodo que representa al proceso hijo

    // Abrir el directorio /proc
    DIR *dir = opendir("/proc");
    if (!dir) {
        perror("No se pudo abrir /proc");
        return 1;
    }

    struct dirent *entry;

    // Recorrer todas las entradas de /proc
    while ((entry = readdir(dir)) != NULL) {

        // Verificar que el nombre sea un numero
        char *name = entry->d_name;
        int is_number = 1;

        for (int i = 0; name[i]; i++) {
            if (!isdigit((unsigned char)name[i])) {
                is_number = 0;
                break;
            }
        }

        if (!is_number)
            continue;

        pid_t pid = (pid_t)atoi(name);

        // Leer el archivo maps del proceso
        char *content = read_maps_file(pid);
        // Crear el nodo correspondiente al proceso
        Node *nuevo = create_node(pid, content);

        if (nuevo) {
            if (pid == parent_pid) {
                // Nodo del proceso padre
                parent_node = nuevo;
                append_node(&head, nuevo);
            } else if (pid == child_pid) {
                // Nodo del proceso hijo
                child_node = nuevo;
            } else {
                // Cualquier otro proceso
                append_node(&head, nuevo);
            }
        }

        // Se libera el contenido
        free(content);
    }

    closedir(dir);

    // Enlazar el padre con su hijo
    if (parent_node && child_node) {
        parent_node->child = child_node;
    }

    // Comparar el contenido de maps del padre y del hijo
    if (parent_node && child_node && parent_node->maps_content && child_node->maps_content) {

        if (strcmp(parent_node->maps_content, child_node->maps_content) == 0) {
            printf("\nEl proceso padre y el hijo tienen el MISMO contenido en maps\n");
        } else {
            printf("\nEl proceso padre y el hijo tienen DIFERENTE contenido en maps\n");
        }
    }

    // Imprimir toda la estructura
    print_list(head);

    // Terminar el proceso hijo de forma controlada
    printf("\nTerminando proceso hijo (%d)\n", child_pid);
    kill(child_pid, SIGTERM);


    // Liberar memoria dinámica
    free_list(head);

    return 0;
}
