#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>

#define PUERTO 8000
#define MAX_LINEA 256
#define MAX_PREGUNTAS 50
#define PREGUNTAS_POR_TEST 10
#define MAX_CLIENTES 10

volatile int clientes_activos = 0;  // Contador global

typedef struct {
    char pregunta[128];
    char incisoA[64];
    char incisoB[64];
    char incisoC[64];
    char correcta;
} Pregunta;

typedef struct {
    char nombre[64];
    char matricula[16];
    char carrera[64];
    char genero[16];
    int edad;
    int semestre;
} Alumno;

void manejadorHijo(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0)
        clientes_activos--;
}

void enviarPregunta(int sockfd, Pregunta *p) {
    send(sockfd, p, sizeof(Pregunta), 0);
}

int cargarPreguntas(const char *archivo, Pregunta preguntas[]) {
    FILE *f = fopen(archivo, "r");
    if (!f) return -1;
    int i = 0;
    char linea[MAX_LINEA];
    while (fgets(linea, sizeof(linea), f) && i < MAX_PREGUNTAS) {
        linea[strcspn(linea, "\n")] = 0;
        sscanf(linea, "%127[^|]|%63[^|]|%63[^|]|%63[^|]|%c",
               preguntas[i].pregunta, preguntas[i].incisoA,
               preguntas[i].incisoB, preguntas[i].incisoC,
               &preguntas[i].correcta);
        i++;
    }
    fclose(f);
    return i;
}

void manejarCliente(int sockfd) {
    Alumno alumno;
    char buffer[MAX_LINEA];
    Pregunta preguntas[MAX_PREGUNTAS];
    int total, correctas = 0;

    recv(sockfd, &alumno, sizeof(alumno), 0);
    recv(sockfd, buffer, sizeof(buffer), 0);  // test elegido
    char testInicial[32];
    strncpy(testInicial, buffer, sizeof(testInicial));

    printf("\n[+] Cliente conectado\n");
    printf("Nombre: %s\nMatrícula: %s\nCarrera: %s\nGénero: %s\nEdad: %d\nSemestre: %d\n",
           alumno.nombre, alumno.matricula, alumno.carrera,
           alumno.genero, alumno.edad, alumno.semestre);
    printf("Test Inicial Elegido: %s\n\n", testInicial);

    char archivo[64];
    snprintf(archivo, sizeof(archivo), "Test%s.txt", testInicial);
    total = cargarPreguntas(archivo, preguntas);
    if (total < PREGUNTAS_POR_TEST) {
        perror("No hay suficientes preguntas");
        close(sockfd); exit(1);
    }

    int indices[PREGUNTAS_POR_TEST];
    for (int i = 0; i < PREGUNTAS_POR_TEST;) {
        int r = rand() % total;
        int repetido = 0;
        for (int j = 0; j < i; j++)
            if (indices[j] == r) { repetido = 1; break; }
        if (!repetido) indices[i++] = r;
    }

    for (int i = 0; i < PREGUNTAS_POR_TEST; i++) {
        printf("[Test Inicial] Pregunta %d: %s\n", i + 1, preguntas[indices[i]].pregunta);
        enviarPregunta(sockfd, &preguntas[indices[i]]);
        recv(sockfd, buffer, 2, 0);
        printf("Respuesta del cliente: %c - Correcta: %c\n", toupper(buffer[0]), preguntas[indices[i]].correcta);
        if (toupper(buffer[0]) == preguntas[indices[i]].correcta)
            correctas++;
    }

    if (correctas >= 6) {
        send(sockfd, "APROBADO", 8, 0);
        printf("[✓] Cliente APROBÓ el test inicial con %d/10\n", correctas);
    } else {
        send(sockfd, "NO APROBADO", 11, 0);
        printf("[✗] Cliente NO APROBÓ el test inicial (%d/10). Desconectando...\n", correctas);
        close(sockfd); exit(0);
    }

    const char *materias[] = {"Matematicas", "Ingles", "Espanol"};
    int aciertos[3] = {0};

    for (int m = 0; m < 3; m++) {
        printf("\n[→] Iniciando examen de %s\n", materias[m]);
        snprintf(archivo, sizeof(archivo), "Test%s.txt", materias[m]);
        total = cargarPreguntas(archivo, preguntas);
        if (total < PREGUNTAS_POR_TEST) continue;

        for (int i = 0; i < PREGUNTAS_POR_TEST;) {
            int r = rand() % total;
            int repetido = 0;
            for (int j = 0; j < i; j++)
                if (indices[j] == r) { repetido = 1; break; }
            if (!repetido) indices[i++] = r;
        }

        for (int i = 0; i < PREGUNTAS_POR_TEST; i++) {
            printf("[%s] Pregunta %d: %s\n", materias[m], i + 1, preguntas[indices[i]].pregunta);
            enviarPregunta(sockfd, &preguntas[indices[i]]);
            recv(sockfd, buffer, 2, 0);
            printf("Respuesta del cliente: %c - Correcta: %c\n", toupper(buffer[0]), preguntas[indices[i]].correcta);
            if (toupper(buffer[0]) == preguntas[indices[i]].correcta)
                aciertos[m]++;
        }
        printf("[✓] Resultado %s: %d/10\n", materias[m], aciertos[m]);
    }

    snprintf(buffer, sizeof(buffer),
             "\tKARDEX FINAL\n\n\tAlumno: %s\n\tMatrícula: %s\n\tCarrera: %s\n\n"
             "\tMatemáticas: %d/10\n\tInglés: %d/10\n\tEspañol: %d/10",
             alumno.nombre, alumno.matricula, alumno.carrera,
             aciertos[0], aciertos[1], aciertos[2]);

    send(sockfd, buffer, strlen(buffer), 0);
    printf("\n[✓] Kardex enviado al cliente. Desconectando...\n");
    close(sockfd);
}

int main() {
    srand(time(NULL));
    int servidor, cliente;
    struct sockaddr_in servidor_addr, cliente_addr;
    socklen_t cliente_len = sizeof(cliente_addr);

    signal(SIGCHLD, manejadorHijo);  // Capturar hijos terminados

    servidor = socket(AF_INET, SOCK_STREAM, 0);
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_port = htons(PUERTO);
    servidor_addr.sin_addr.s_addr = INADDR_ANY;

    bind(servidor, (struct sockaddr*)&servidor_addr, sizeof(servidor_addr));
    listen(servidor, 5);
    printf("[Servidor] Esperando conexiones en el puerto %d...\n", PUERTO);

    while (1) {
        if (clientes_activos >= MAX_CLIENTES) {
            printf("[!] Límite de %d clientes alcanzado. Rechazando conexión temporal...\n", MAX_CLIENTES);
            int tmp = accept(servidor, NULL, NULL);
            close(tmp);
            continue;
        }

        cliente = accept(servidor, (struct sockaddr*)&cliente_addr, &cliente_len);
        if (cliente < 0) continue;

        int pid = fork();
        if (pid == 0) {
            printf("[PID %d] Cliente conectado desde %s:%d\n", getpid(),
                   inet_ntoa(cliente_addr.sin_addr), ntohs(cliente_addr.sin_port));
            close(servidor);
            manejarCliente(cliente);
            exit(0);
        } else {
            clientes_activos++;
            close(cliente);
        }
    }

    return 0;
}
