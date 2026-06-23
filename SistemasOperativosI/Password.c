#include <ncurses.h>

int main(){
   char cadena[128];
   initscr();
   erase();
   start_color();
   init_pair(1, COLOR_RED, COLOR_YELLOW);
   init_pair(2, COLOR_BLUE, COLOR_GREEN);
   attron(COLOR_PAIR(1));
   move(12,20);
   printw("Dame tu password: \n");
   noecho(); /*No se ve lo que se escribe*/
   scanw("%s", cadena);
   echo(); /*Ya se ve lo que se escribe*/
   attroff(COLOR_PAIR(1));
   attron(COLOR_PAIR(2));
   mvprintw(13,20, "Tu password es: %s", cadena);
   //printw("Tu password es: %s", cadena);
   attroff(COLOR_PAIR(2));
   refresh();
   getch();
   endwin();
   return 0;
}