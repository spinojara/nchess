#include "topbar.h"

#include <string.h>

#include "window.h"
#include "draw.h"
#include "mainwin.h"
#include "editwin.h"
#include "info.h"

static int selected = 0;

static const char *options[] = { "New  Game", "Settings", "Engines", "Edit Board", "Analysis" };

static const int noptions = sizeof(options) / sizeof(*options);

static int refreshed = 0;

void topbar_draw(void);

void topbar_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case 'h':
	case KEY_LEFT:
		refreshed = 0;
		selected = (selected + 4) % 5;
		break;
	case '\t':
	case 'l':
	case KEY_RIGHT:
		refreshed = 0;
		selected = (selected + 1) % 5;
		break;
	case 'j':
	case KEY_DOWN:
		place_top(&mainwin);
		break;
		break;
	case KEY_MOUSE:
		if (event->x < 3)
			break;
		int i, x;
		for (i = 0, x = 3; i < noptions; i++) {
			/* We are between two options. */
			if (event->x <= x - 1) {
				i = noptions;
				break;
			}
			x += strlen(options[i]) + 6;
			if (event->x <= x - 3)
				break;
		}
		if (i < noptions) {
			selected = i;
			refreshed = 0;
		}
		else {
			break;
		}
		/* fallthrough */
	case '\n':
		switch (selected) {
		case 0:
			if (gamerunning)
				end_game();
			else
				place_top(&newgame);
			break;
		case 1:
			place_top(&settings);
			break;
		case 2:
#ifndef _WIN32
			place_top(&engines);
#else
			not_supported();
			refreshed = 0;
#endif
			break;
		case 3:
			if (gamerunning)
				break;
			/* Only reset the edited position if mainwin is above editwin. */
			for (int j = 0; j < nwins; j++) {
				if (wins[j] == &mainwin) {
					pos = posd;
					break;
				}
				else if (wins[j] == &editwin)
					break;
			}
			place_top(&editwin);
			break;
		case 4:
#ifndef _WIN32
			place_top(&analysis);
#else
			not_supported();
			refreshed = 0;
#endif
			break;
		}
		break;
	case 'q':
		running = 0;
		break;
	}

	if (!refreshed)
		topbar_draw();
}

void topbar_draw(void) {
	int x, y, i;
	getmaxyx(topbar.win, y, x);
	draw_fill(topbar.win, &cs.border, 0, 0, y, x, NULL);
	for (i = 0, x = 3; i < noptions; i++) {
		set_color(topbar.win, selected == i && wins[0] == &topbar ? &cs.texthl : &cs.text);
		mvwprintw(topbar.win, 1, x, "< %s >", i == 0 && gamerunning ? "Stop Game" : options[i]);
		x += strlen(options[i]) + 6;
	}
	wrefresh(topbar.win);
	refreshed = 1;
}

void topbar_resize(void) {
	wresize(topbar.win, 3, COLS - 10);
	mvwin(topbar.win, 2, 4);

	topbar_draw();
}
