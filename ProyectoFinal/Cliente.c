#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PUERTO 8000
#define MAX_LINEA 256
#define PREGUNTAS_POR_TEST 10

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

void iniciarColores() {
    start_color();
    use_default_colors();

    init_pair(1, COLOR_CYAN, COLOR_BLACK);     // Títulos, Bienvenida, Presione para salir
    init_pair(2, COLOR_WHITE, COLOR_BLACK);    // Subtitulo
    init_pair(3, COLOR_GREEN, -1);             // Cargando, Seleccionar Opcion
    init_pair(4, COLOR_WHITE, -1);             // Frase, Entrada usuario, Kardex
    init_pair(5, COLOR_BLUE, COLOR_BLACK);     // Pregunta
    init_pair(6, COLOR_BLACK, COLOR_WHITE);    // Fondo botones seleccionados
    init_pair(7, COLOR_RED, -1);               // Advertencias o fallos
    init_pair(8, COLOR_BLUE, -1);              // Marcos y Fondos
}

void mostrarPantallaBienvenida() {
    clear();
    curs_set(0);
    attron(COLOR_PAIR(8));
    box(stdscr, 0, 0);
    attroff(COLOR_PAIR(8));

    // Bienvenida
    const char *titulo = "BIENVENIDO AL SISTEMA DE EXAMEN EN LINEA";
    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(2, (COLS - strlen(titulo)) / 2, "%s", titulo);
    attroff(COLOR_PAIR(1) | A_BOLD);

    // Escuela
    const char *subtitulo = "Benemérita Universidad Autónoma de Puebla";
    attron(COLOR_PAIR(1));
    mvprintw(4, (COLS - strlen(subtitulo)) / 2, "%s", subtitulo);
    attroff(COLOR_PAIR(1));

    // Simulación de loading con puntos
    const char *loading = "Cargando plataforma";
    attron(COLOR_PAIR(3));
    int col_loading = (COLS - strlen(loading) - 3) / 2; // -3 por los 3 puntos
    mvprintw(6, col_loading, "%s", loading);
    refresh();
    for (int i = 0; i < 3; i++) {
        printw(".");
        refresh();
        napms(1000); // Espera 1 segundo por punto
    }
    attroff(COLOR_PAIR(3));

    // Frase motivacional centrada
    const char *frase = "\"Tu conocimiento es tu mejor herramienta\"";
    attron(COLOR_PAIR(2) | A_ITALIC);
    mvprintw(9, (COLS - strlen(frase)) / 2, "%s", frase);
    attroff(COLOR_PAIR(2) | A_ITALIC);

    // Información del desarrollador centrada
    const char *devInfo = "Desarrollado por: Brandon Domínguez León y Eric Garcia Gonzalez";
    attron(COLOR_PAIR(5));
    mvprintw(11, (COLS - strlen(devInfo)) / 2, "%s", devInfo);
    attroff(COLOR_PAIR(5));


    //Agregar Logo de Inicio

    const char* figura[] = {
        "   *********   ",
        "  ***  *  ***  ",
        " ***  ***  *** ",
        "***  *****  ***",
        "**    ***    **",
        "**  * *** *  **",
        "** * ***** * **",
        "** * ***** * **",
        "**  *******  **",
        "***  *****  ***",
        " ***  ***   ** ",
        " ****  *  **** ",
        "   *********   "
    };
    int numLineasFigura = sizeof(figura) / sizeof(figura[0]);
    int filaInicio = 15; // Dos líneas después del mensaje de desarrolladores

    attron(COLOR_PAIR(4)); // Puedes elegir el color que prefieras para la figura
    for (int i = 0; i < numLineasFigura; i++) {
        int columna = (COLS - strlen(figura[i])) / 2;
        mvprintw(filaInicio + i, columna, "%s", figura[i]);
    }
    attroff(COLOR_PAIR(4));

    // Instrucción para continuar
    attron(A_DIM);
    mvprintw(LINES - 3, (COLS - 36) / 2, "Presiona cualquier tecla para continuar...");
    attroff(A_DIM);

    refresh();
    getch();
    clear();
}


void dibujarMarco() {
    int ancho = COLS;
    int alto = LINES;

    attron(COLOR_PAIR(8));
    mvaddch(0, 0, ACS_ULCORNER);
    mvaddch(0, ancho - 1, ACS_URCORNER);
    mvaddch(alto - 1, 0, ACS_LLCORNER);
    mvaddch(alto - 1, ancho - 1, ACS_LRCORNER);
    for (int i = 1; i < ancho - 1; i++) {
        mvaddch(0, i, ACS_HLINE);
        mvaddch(alto - 1, i, ACS_HLINE);
    }
    for (int i = 1; i < alto - 1; i++) {
        mvaddch(i, 0, ACS_VLINE);
        mvaddch(i, ancho - 1, ACS_VLINE);
    }
    attroff(COLOR_PAIR(8));

    // Logo
    const char *logo[] = {
          " *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *",
          "   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  ",
          " *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *",
          "   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  ",
          "----------------------------------------------------------------------------------------------------------------------------------",
          "  ****   *   *  *****  *****  *************************  *****    *****    *****  *************************  *****  *****  *****  ",
          "  *  *   *   *  *   *  *   *  **-----*-*-----*-*-----**  *        *   *      *    **-----*-*-----*-*-----**  *      *      *      ",
          "  *****  *   *  *****  *****  *-*---*---*---*---*---*-*  *****    *   *      *    *-*---*---*---*---*---*-*  ***    *      *      ",
          "  *   *  *   *  *   *  *      *--*-*-----*-*-----*-*--*      *    *   *      *    *--*-*-----*-*-----*-*--*  *      *      *      ",
          "  *****   ***   *   *  *      *************************  *****.   *****.   *****  *************************  *      *****  *****  ",
          "----------------------------------------------------------------------------------------------------------------------------------",
          "   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  ",
          " *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *",
          " *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *",
          "   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *  "
    };
    int numLineas = sizeof(logo) / sizeof(logo[0]);
    int startY = alto - numLineas - 2;
    int startX = (ancho - strlen(logo[0])) / 2;

    attron(COLOR_PAIR(4) | A_BOLD);
    for (int i = 0; i < numLineas; i++) {
        mvprintw(startY + i, startX, "%s", logo[i]);
    }
    attroff(COLOR_PAIR(4) | A_BOLD);
}

void mostrarTitulo(const char *titulo) {
    attron(COLOR_PAIR(7) | A_BOLD | A_UNDERLINE);
    mvprintw(1, (COLS - strlen(titulo)) / 2, "%s", titulo);
    attroff(COLOR_PAIR(7) | A_BOLD | A_UNDERLINE);
}

void capturarAlumno(Alumno *a) {
    clear(); dibujarMarco(); mostrarTitulo("Registro de Alumno");

    attron(COLOR_PAIR(4));
    move(4, 6); printw("Nombre: "); echo(); getnstr(a->nombre, 63);
    move(5, 6); printw("Matrícula: "); getnstr(a->matricula, 15);
    move(6, 6); printw("Carrera: "); getnstr(a->carrera, 63);
    move(7, 6); printw("Género (M/F): "); getnstr(a->genero, 15);
    move(8, 6); printw("Edad (Número): "); scanw("%d", &a->edad);
    move(9, 6); printw("Semestre (Número): "); scanw("%d", &a->semestre);
    noecho();
    attroff(COLOR_PAIR(4));
}

void elegirTest(char *test) {
    char opciones[3][16] = {"Visual", "Logico", "Razonamiento"};
    int opcion = 0, tecla;

    while (1) {
        clear(); dibujarMarco(); mostrarTitulo("Selecciona el Test Inicial");
        for (int i = 0; i < 3; i++) {
            if (i == opcion)
                attron(COLOR_PAIR(6) | A_BOLD);
            else
                attron(COLOR_PAIR(3));
            mvprintw(6 + i * 2, (COLS - 20) / 2, ">> %s <<", opciones[i]);
            if (i == opcion)
                attroff(COLOR_PAIR(6) | A_BOLD);
            else
                attroff(COLOR_PAIR(3));
        }

        tecla = getch();
        if (tecla == KEY_UP) opcion = (opcion + 2) % 3;
        else if (tecla == KEY_DOWN) opcion = (opcion + 1) % 3;
        else if (tecla == '\n') break;
    }

    strcpy(test, opciones[opcion]);
}

char responderPregunta(Pregunta *p, int num) {
    clear(); dibujarMarco();
    mostrarTitulo("Responde la Pregunta");

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(4, 4, "Pregunta %d:", num);
    mvprintw(6, 6, "%s", p->pregunta);
    attroff(COLOR_PAIR(1) | A_BOLD);

    attron(COLOR_PAIR(1));
    mvprintw(8, 8, "A) %s", p->incisoA);
    mvprintw(9, 8, "B) %s", p->incisoB);
    mvprintw(10, 8, "C) %s", p->incisoC);
    attroff(COLOR_PAIR(1));

    attron(COLOR_PAIR(5));
    mvprintw(12, 6, "Tu respuesta (A/B/C): ");
    echo();
    char resp = toupper(getch());
    noecho();
    attroff(COLOR_PAIR(5));

    return resp;
}

int main() {
    initscr();
    cbreak(); keypad(stdscr, TRUE);
    curs_set(0);
    iniciarColores();

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in servidor;
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(PUERTO);
    servidor.sin_addr.s_addr = inet_addr("172.26.165.10");

    if (connect(sockfd, (struct sockaddr*)&servidor, sizeof(servidor)) < 0) {
        endwin(); perror("Error al conectar con el servidor"); exit(1);
    }

    Alumno alumno;
    char test[32], buffer[MAX_LINEA];
    Pregunta pregunta;

    mostrarPantallaBienvenida();
    capturarAlumno(&alumno);
    elegirTest(test);

    send(sockfd, &alumno, sizeof(alumno), 0);
    send(sockfd, test, sizeof(test), 0);

    for (int i = 0; i < PREGUNTAS_POR_TEST; i++) {
        recv(sockfd, &pregunta, sizeof(pregunta), 0);
        char r = responderPregunta(&pregunta, i + 1);
        send(sockfd, &r, 1, 0);
    }

    recv(sockfd, buffer, sizeof(buffer), 0);
    buffer[11] = '\0';

    if (strncmp(buffer, "APROBADO", 8) == 0) {
        clear(); dibujarMarco(); mostrarTitulo("¡Test Inicial Aprobado!");
        mvprintw(6, 6, "Iniciando examen final...");
        refresh(); sleep(2);

        for (int m = 0; m < 3; m++) {
            for (int i = 0; i < PREGUNTAS_POR_TEST; i++) {
                recv(sockfd, &pregunta, sizeof(pregunta), 0);
                char r = responderPregunta(&pregunta, i + 1);
                send(sockfd, &r, 1, 0);
            }
        }

        memset(buffer, 0, sizeof(buffer));
        recv(sockfd, buffer, sizeof(buffer), 0);
        clear(); dibujarMarco(); mostrarTitulo("Kardex Final");
        attron(COLOR_PAIR(4));
        mvprintw(5, 4, "%s", buffer);
        attroff(COLOR_PAIR(4));
    } else {
        clear(); dibujarMarco(); mostrarTitulo("Test No Aprobado");
        attron(COLOR_PAIR(7) | A_BOLD);
        mvprintw(6, 6, "No aprobaste el test inicial. Fin del examen.");
        attroff(COLOR_PAIR(7));
    }

    attron(COLOR_PAIR(1) | A_BOLD);
    mvprintw(LINES - 2, 4, "Presiona cualquier tecla para salir...");
    attroff(COLOR_PAIR(1));
    getch();

    endwin();
    close(sockfd);
    return 0;
}
