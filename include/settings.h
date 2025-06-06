#ifndef SETTINGS_H
#define SETTINGS_H

#include <curses.h>

void settings_event(chtype ch, MEVENT *event);

void settings_resize(void);

int settings_readconfig(void);

int settings_writeconfig(void);

#endif
