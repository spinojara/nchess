#ifndef EDITENGINE_H
#define EDITENGINE_H

#include <ncurses.h>

void editengine_event(chtype ch, MEVENT *event);

void editengine_resize(void);

void editengine_init(void);

#endif
