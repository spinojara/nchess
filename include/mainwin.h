#ifndef MAINWIN_H
#define MAINWIN_H

#include <ncurses.h>

void mainwin_event(chtype ch, MEVENT *event);

void mainwin_resize(void);

void mainwin_init(void);

#endif
