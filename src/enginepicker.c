#include "enginepicker.h"

#include <string.h>

#include "color.h"
#include "draw.h"
#include "window.h"
#include "newgame.h"
#include "engines.h"

static int refreshed = 0;

static int selected = 0;

static const struct uciengine **engine = NULL;

void enginepicker_draw();

void enginepicker_event(chtype ch, MEVENT *event) {
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
				place_top(&newgame);
				*engine = NULL;
			}
		}
		if (2 <= event->y && event->y <= nengines + 1) {
			if (2 <= event->x && event->x < 26) {
				refreshed = 1;
				selected = event->y - 1;
				*engine = &uciengines[selected - 1];
				place_top(&newgame);
			}
		}
		break;
	case KEY_ENTER:
	case '\n':
		if (selected == 0)
			*engine = NULL;
		else
			*engine = &uciengines[selected - 1];
		place_top(&newgame);
		refreshed = 1;
		break;
	case KEY_ESC:
	case 'q':
		place_top(&newgame);
		refreshed = 1;
		selected = 0;
		break;
	case 'k':
	case KEY_UP:
		if (0 < selected) {
			refreshed = 0;
			selected--;
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
		enginepicker_draw();
}

void enginepicker_setup(const struct uciengine **e) {
	engine = e;
	selected = 0;
	if (!engine)
		return;
	for (int i = 0; i < nengines; i++)
		if (*engine == &uciengines[i])
			selected = i + 1;
}

void enginepicker_draw(void) {
	int x, y;
	getmaxyx(enginepicker.win, y, x);
	draw_border(enginepicker.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	set_color(enginepicker.win, &cs.border);
	mvwhline(enginepicker.win, 0, 28, ACS_HLINE, 2);
	set_color(enginepicker.win, selected == 0 ? &cs.texthl : &cs.text);
	mvwaddstr(enginepicker.win, 1, 5, "< Human >");
	if (!nengines) {
		set_color(enginepicker.win, &cs.text);
		mvwaddstr(enginepicker.win, 2, 4, "No Engines Available");
	}
	char name[24];
	for (int i = 0; i < nengines; i++) {
		set_color(enginepicker.win, selected == i + 1 ? &cs.texthl : &cs.text);
		snprintf(name, 24, "%s", uciengines[i].name);
		if (strlen(name) == 23) {
			name[19] = '.';
			name[20] = '.';
			name[21] = '.';
			name[22] = '\0';
		}
		mvwaddstr(enginepicker.win, 2 + i, 4, name);
	}

	wrefresh(enginepicker.win);
	refreshed = 1;
}

void enginepicker_resize(void) {
	wresize(enginepicker.win, 35, 30);
	mvwin(enginepicker.win, 7, 7);

	enginepicker_draw();
}
