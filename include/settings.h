#ifndef SETTINGS_H
#define SETTINGS_H

#include <ncurses.h>

void settings_event(chtype ch, MEVENT *event);

void settings_resize(void);

#endif
