#include "engines.h"

#include "window.h"
#include "color.h"
#include "draw.h"

static int refreshed = 0;

void engines_draw();

void engines_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 'r':
		refreshed = 0;
		break;
	}

	if (!refreshed)
		engines_draw();
}

void engines_draw(void) {
	int x, y;
	getmaxyx(engines.win, y, x);
	draw_border(engines.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	wrefresh(engines.win);
	refreshed = 1;
}

void engines_resize(void) {
	wresize(engines.win, 35, 30);
	mvwin(engines.win, 7, 7);

	engines_draw();
}
