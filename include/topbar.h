#ifndef TOPBAR_H
#define TOPBAR_H

#include <ncurses.h>

void topbar_event(chtype ch, MEVENT *event);

void topbar_resize(void);

#endif
