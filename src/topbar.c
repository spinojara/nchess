#include "topbar.h"

#include <string.h>

#include "window.h"
#include "draw.h"

static int selected = 0;

static const char *options[] = { "New Game", "Settings", "Engines", "Edit Board", "Analysis" };

static const int noptions = sizeof(options) / sizeof(*options);

static int refreshed = 0;

void topbar_draw(void);

void topbar_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 'r':
		refreshed = 0;
		break;
	case 'h':
	case KEY_LEFT:
		if (selected > 0) {
			refreshed = 0;
			selected--;
		}
		break;
	case 'l':
	case KEY_RIGHT:
		if (selected < noptions - 1) {
			refreshed = 0;
			selected++;
		}
		break;
	case 'j':
	case KEY_DOWN:
		place_top(&mainwin);
		break;
	case 'i':
		place_top(&engines);
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
		if (selected == 2)
			place_top(&engines);
	}

	if (!refreshed)
		topbar_draw();
}

void topbar_draw(void) {
	int x, y;
	getmaxyx(topbar.win, y, x);
	draw_fill(topbar.win, &cs.border, 0, 0, y, x);
	for (int i = 0, x = 3; i < noptions; i++) {
		set_color(topbar.win, selected == i && wins[0] == &topbar ? &cs.texthl : &cs.text);
		mvwprintw(topbar.win, 1, x, "< %s >", options[i]);
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