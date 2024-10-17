#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>

struct window {
	WINDOW *win;
	void (*event)(chtype, MEVENT *);
};

extern struct window topbar, mainwin, editwin, newgame, settings, engines, editengine;
extern struct window *wins[];
extern const int nwins;

void window_init(void);

void window_resize(void);

void place_top(struct window *win);

#endif
