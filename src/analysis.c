#include "analysis.h"

#include <string.h>

#include "color.h"
#include "draw.h"
#include "window.h"
#include "engines.h"
#include "mainwin.h"

static int refreshed = 0;

static int selected = 0;

void analysis_draw();

void analysis_event(chtype ch, MEVENT *event) {
	if (selected >= nengines + 1)
		selected = 0;
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_MOUSE:
		if (1 == event->y) {
			if (5 <= event->x && event->x < 22) {
				refreshed = 1;
				selected = 0;
				end_analysis();
				place_top(&mainwin);
			}
		}
		if (2 <= event->y && event->y <= nengines + 1) {
			if (2 <= event->x && event->x < 26) {
				refreshed = 1;
				selected = event->y - 1;
				start_analysis(&uciengines[selected - 1]);
				place_top(&mainwin);
			}
		}
		break;
	case KEY_ENTER:
	case '\n':
		if (selected == 0) {
			end_analysis();
		}
		else if (selected && selected - 1 < nengines) {
			start_analysis(&uciengines[selected - 1]);
		}
		place_top(&mainwin);
		refreshed = 1;
		break;
	case KEY_ESC:
	case 'q':
		place_top(&topbar);
		selected = 0;
		break;
	case 'k':
	case KEY_UP:
		if (0 < selected) {
			refreshed = 0;
			selected--;
		}
		else {
			refreshed = 1;
			place_top(&topbar);
		}
		break;
	case '\t':
		selected = (selected + 1) % (nengines + 1);
		refreshed = 0;
		break;
	case 'j':
	case KEY_DOWN:
		if (selected < nengines) {
			refreshed = 0;
			selected++;
		}
		break;
	}

	if (!refreshed)
		analysis_draw();
}

void analysis_draw(void) {
	int x, y;
	getmaxyx(analysis.win, y, x);
	draw_border(analysis.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	set_color(analysis.win, selected == 0 ? &cs.texthl : &cs.text);
	mvwaddstr(analysis.win, 1, 5, "< Stop Analysis >");
	if (!nengines) {
		set_color(analysis.win, &cs.text);
		mvwaddstr(analysis.win, 2, 4, "No Engines Available");
	}
	char name[24];
	for (int i = 0; i < nengines; i++) {
		set_color(analysis.win, selected == i + 1 ? &cs.texthl : &cs.text);
		snprintf(name, 24, "%s", uciengines[i].name);
		if (strlen(name) == 23) {
			name[19] = '.';
			name[20] = '.';
			name[21] = '.';
			name[22] = '\0';
		}
		mvwaddstr(analysis.win, 2 + i, 4, name);
	}

	wrefresh(analysis.win);
	refreshed = 1;
}

void analysis_resize(void) {
	wresize(analysis.win, 35, 30);
	mvwin(analysis.win, 7, 7);

	analysis_draw();
}
