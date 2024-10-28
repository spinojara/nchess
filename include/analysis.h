#ifndef ANALYSIS_H
#define ANALYSIS_H

#include <curses.h>

void analysis_event(chtype ch, MEVENT *event);

void analysis_resize(void);

#endif
