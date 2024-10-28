#ifndef EDITWIN_H
#define EDITWIN_H

#include <curses.h>

extern struct position pos;

void editwin_event(chtype ch, MEVENT *event);

void editwin_resize(void);

#endif
