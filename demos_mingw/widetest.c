/*
   Wide Character Display Test
   2020-12-10 v1.11

   OS       : Windows 10 (version 1909) (64bit)
   DevTools : MSYS2/MinGW-w64 (64bit) (gcc version 10.2.0 (Rev1, Built by MSYS2 project))
   Compile  : gcc -g -O2 -Wall -Wextra -o widetest.exe widetest.c
*/

#include <pdcurses.h>
#include <stdlib.h>

int main(void)
{
    int i;
    const char *str = "1234567890あいうえおかきくけこさしすせそ";

    /* initialize */
    initscr();
    noecho();

    /* set color */
    if (!has_colors()) {
        mvaddstr(0, 0, "color not supported.");
        getch();
        exit(0);
    }
    start_color();
    use_default_colors();
    init_color( 1,    0,    0,  500);
    init_color( 2,    0,  500,    0);
    init_color( 3,    0,  500,  500);
    init_color( 4,  500,    0,    0);
    init_color( 5,  500,    0,  500);
    init_color( 6,  500,  500,    0);
    init_color( 7,  752,  752,  752);
    init_color( 8,  500,  500,  500);
    init_color( 9,    0,    0, 1000);
    init_color(10,    0, 1000,    0);
    init_color(11,    0, 1000, 1000);
    init_color(12, 1000,    0,    0);
    init_color(13, 1000,    0, 1000);
    init_color(14, 1000, 1000,    0);
    init_color(15, 1000, 1000, 1000);
    for (i = 1; i < 16; i++) {
        init_pair(i, i, -1);
    }

    /* display */
    clear();
    for (i = 0; i < LINES; i++) {
        attrset(COLOR_PAIR(i % 16));
        mvaddstr(i,  0, str);
        attrset(COLOR_PAIR((i + 1) % 16));
        mvaddstr(i, 25, str);
    }
    refresh();

    /* wait for key input */
    getch();

    /* finished */
    endwin();
    return 0;
}
