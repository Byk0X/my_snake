/* screen.h */
#include <ncurses.h>

void init_screen();
void done_screen();
void gotoyx(int y, int x);
int ngetch();
void getscreensize(int& y, int& x);
