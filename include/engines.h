#ifndef ENGINES_H
#define ENGINES_H

#include <ncurses.h>

void engines_event(chtype ch, MEVENT *event);

void engines_resize(void);

#endif
