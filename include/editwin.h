#ifndef EDITWIN_H
#define EDITWIN_H

#include <ncurses.h>

extern struct position pos;

void editwin_event(chtype ch, MEVENT *event);

void editwin_resize(void);

#endif
