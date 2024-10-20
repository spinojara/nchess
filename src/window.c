#include "window.h"

#include <stdlib.h>
#include <stdarg.h>

#include "color.h"
#include "topbar.h"
#include "mainwin.h"
#include "draw.h"
#include "engines.h"
#include "editengine.h"
#include "analysis.h"
#include "editwin.h"
#include "settings.h"

#define MINLINES 56
#define MINCOLS 117

struct window topbar, mainwin, editwin, newgame, settings, engines, editengine, analysis;

struct window *wins[] = { &topbar, &mainwin, &editwin, &newgame, &settings, &engines, &editengine, &analysis };

const int nwins = sizeof(wins) / sizeof(*wins);

void die(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	endwin();
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(1);
}

void window_init(void) {
	topbar.win = newwin(0, 0, 0, 0);
	keypad(topbar.win, TRUE);
	topbar.event = &topbar_event;

	mainwin.win = newwin(0, 0, 0, 0);
	keypad(mainwin.win, TRUE);
	mainwin.event = &mainwin_event;
	wtimeout(mainwin.win, 25);

	editwin.win = newwin(0, 0, 0, 0);
	keypad(editwin.win, TRUE);
	editwin.event = &editwin_event;

	newgame.win = newwin(0, 0, 0, 0);
	keypad(newgame.win, TRUE);

	settings.win = newwin(0, 0, 0, 0);
	keypad(settings.win, TRUE);
	settings.event = &settings_event;

	engines.win = newwin(0, 0, 0, 0);
	keypad(engines.win, TRUE);
	engines.event = &engines_event;

	editengine.win = newwin(0, 0, 0, 0);
	keypad(editengine.win, TRUE);
	editengine.event = &editengine_event;

	analysis.win = newwin(0, 0, 0, 0);
	keypad(analysis.win, TRUE);
	analysis.event = &analysis_event;
}

void window_resize(void) {
	if (LINES < MINLINES || COLS < MINCOLS)
		die("error: terminal needs to be of size at least %dx%d\n", MINLINES, MINCOLS);
	wbkgd(stdscr, cs.bg.attr);
	draw_fill(stdscr, &cs.bg, 0, 0, LINES, COLS);
	draw_border(stdscr, &cs.bg, &cs.border, &cs.bordershadow, 1, 1, 2, LINES - 2, COLS - 4);

	topbar_resize();
	mainwin_resize();
	engines_resize();
	editengine_resize();
	analysis_resize();
	editwin_resize();
	settings_resize();

	wresize(newgame.win, LINES - 8, COLS - 10);
	mvwin(newgame.win, 5, 4);

	touchwin(stdscr);
	wrefresh(stdscr);
	for (int i = nwins - 1; i >= 0; i--) {
		touchwin(wins[i]->win);
		wrefresh(wins[i]->win);
	}
}

#ifndef NDEBUG
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
	else if (win == &editengine)
		printf("editengine\n");
	else
		printf("unknown\n");
}

void print_order(void) {
	printf("order:\n");
	for (int i = 0; i < nwins; i++)
		print_window(wins[i]);
}
#endif

void place_top(struct window *win) {
	if (win == wins[0])
		return;
	/* If we are putting topbar on top, put mainwin or editwin on top first,
	 * so that it comes second.
	 */
	if (win == &topbar) {
		int i;
		for (i = 0; i < nwins; i++)
			if (wins[i] == &mainwin || wins[i] == &editwin)
				break;
		place_top(wins[i]);
	}
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
