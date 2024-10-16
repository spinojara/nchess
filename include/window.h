#ifndef WINDOW_H
#define WINDOW_H

#include <ncurses.h>

struct window {
	WINDOW *win;
	void (*event)(chtype, MEVENT *);
};

extern struct window topbar, mainwin, editwin, newgame, settings, engines;
extern struct window *wins[6];
extern const int nwins;

void window_init(void);

void window_resize(void);

void place_top(struct window *win);

#endif
