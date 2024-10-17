#include "window.h"

#include <stdlib.h>

#include "color.h"
#include "topbar.h"
#include "mainwin.h"
#include "draw.h"
#include "engines.h"
#include "editengine.h"

#define MINLINES 56
#define MINCOLS 117

struct window topbar, mainwin, editwin, newgame, settings, engines, editengine;

struct window *wins[] = { &topbar, &mainwin, &editwin, &newgame, &settings, &engines, &editengine };

const int nwins = sizeof(wins) / sizeof(*wins);

void window_init(void) {
	topbar.win = newwin(0, 0, 0, 0);
	keypad(topbar.win, TRUE);
	topbar.event = &topbar_event;

	mainwin.win = newwin(0, 0, 0, 0);
	keypad(mainwin.win, TRUE);
	mainwin.event = &mainwin_event;

	editwin.win = newwin(0, 0, 0, 0);
	keypad(editwin.win, TRUE);

	newgame.win = newwin(0, 0, 0, 0);
	keypad(newgame.win, TRUE);

	settings.win = newwin(0, 0, 0, 0);
	keypad(settings.win, TRUE);

	engines.win = newwin(0, 0, 0, 0);
	keypad(engines.win, TRUE);
	engines.event = &engines_event;

	editengine.win = newwin(0, 0, 0, 0);
	keypad(editengine.win, TRUE);
	editengine.event = &editengine_event;
}

void window_resize(void) {
	if (LINES < MINLINES || COLS < MINCOLS) {
		endwin();
		fprintf(stderr, "error: terminal needs to be of size at least %dx%d\n", MINLINES, MINCOLS);
		exit(1);
	}
	draw_fill(stdscr, &cs.bg, 0, 0, LINES, COLS);
	draw_border(stdscr, &cs.bg, &cs.border, &cs.bordershadow, 1, 1, 2, LINES - 2, COLS - 4);

	topbar_resize();
	mainwin_resize();
	engines_resize();
	editengine_resize();

	wresize(editwin.win, LINES - 8, COLS - 10);
	mvwin(editwin.win, 5, 4);

	wresize(newgame.win, LINES - 8, COLS - 10);
	mvwin(newgame.win, 5, 4);

	wresize(settings.win, LINES - 8, COLS - 10);
	mvwin(settings.win, 5, 4);

	touchwin(stdscr);
	wrefresh(stdscr);
	for (int i = nwins - 1; i >= 0; i--) {
		touchwin(wins[i]->win);
		wrefresh(wins[i]->win);
	}
}

void print_window(struct window *win) {
	if (win == &topbar)
		printf("topbar\n");
	else if (win == &mainwin)
		printf("mainwin\n");
	else if (win == &engines)
		printf("engines\n");
	else if (win == &settings)
		printf("settings\n");
	else if (win == &newgame)
		printf("newgame\n");
	else if (win == &editwin)
		printf("editwin\n");
}

void place_top(struct window *win) {
	if (win == wins[0])
		return;
	/* If we are putting topbar on top, put mainwin on top first, so that
	 * it comes second.
	 */
	if (win == &topbar)
		place_top(&mainwin);
	for (int i = nwins - 1, flag = 0; i >= 1; i--) {
		if (wins[i] != win && !flag)
			continue;
		flag = 1;
		struct window *w = wins[i - 1];
		wins[i - 1] = wins[i];
		wins[i] = w;
	}
	/* Topbar needs to be redrawn if it was previously on top. */
	if (wins[1] == &topbar)
		topbar.event(0, NULL);
	win->event(0, NULL);
}
