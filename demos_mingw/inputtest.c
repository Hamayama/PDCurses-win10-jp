/*
   Key and Mouse Input Test
   2020-11-1 v1.00

   OS       : Windows 10 (version 1909) (64bit)
   DevTools : MSYS2/MinGW-w64 (64bit) (gcc version 10.2.0 (Rev1, Built by MSYS2 project))
   Compile  : gcc -g -O2 -Wall -Wextra -o test_input.exe test_input.c
*/

#include <pdcurses.h>
#include <windows.h>
#include <stdlib.h>

int main(void)
{
    HANDLE hout;
    int ch;
    int ctrl;
    MEVENT mouse_event;

    /* initialize */
    initscr();
    noecho();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);

    /* don't use curses screen */
    hout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleActiveScreenBuffer(hout);

    /* start-up message */
    printf("Please try key and mouse input.\n");
    printf("( [q] key to exit )\n");

    /* input loop */
    for (;;) {
        /* wait for key input */
        ch = getch();

        /* get modifier key state */
        ctrl = PDC_get_key_modifiers();

        /* resize event */
        if (ch == 0x222) {
            continue;
        }

        /* mouse event */
        if (ch == 0x21b) {
            nc_getmouse(&mouse_event);
            printf("mouse event : x=%d y=%d bstate=0x%lx\n", mouse_event.x, mouse_event.y, mouse_event.bstate);
            continue;
        }

        /* key event */
        printf("key event : char=0x%x ctrl=%x\n", ch, ctrl);

        /* [q] key to exit */
        if (ch == 0x51 || ch == 0x71) { break; }
    }

    /* finished */
    endwin();
}
