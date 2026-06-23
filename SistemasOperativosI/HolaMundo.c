#include <ncurses.h>
int main(){
   initscr();
   printw("Hola Linux");
   refresh();
   getch();
   endwin();
   return 0;
}