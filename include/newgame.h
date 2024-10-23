#ifndef NEWGAME_H
#define NEWGAME_H

#include <ncurses.h>

void newgame_event(chtype ch, MEVENT *event);

void newgame_resize(void);

void newgame_init(void);

#endif
