#include "settings.h"

#include "window.h"
#include "color.h"
#include "draw.h"
#include "mainwin.h"

static int refreshed = 0;

static int selected = 0;

static const char *options[] = { "Flip Board", };

static int noptions = sizeof(options) / sizeof(*options);

void settings_draw();

void settings_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_MOUSE:
		if (1 <= event->y && event->y <= noptions && 2 <= event->x && event->x < 26) {
			refreshed = 1;
			selected = event->y - 1;

		}
		else {
			break;
		}
		/* fallthrough */
	case KEY_ENTER:
	case '\n':
		refreshed = 1;
		switch (selected) {
		case 0:
			flipped = !flipped;
			place_top(&mainwin);
			refreshed = 1;
			break;
		}
		break;
	case KEY_ESC:
		refreshed = 1;
		place_top(&topbar);
		break;
	case 'k':
	case KEY_UP:
		if (0 < selected) {
			refreshed = 0;
			selected--;
		}
		break;
	case '\t':
		selected = (selected + 1) % noptions;
		refreshed = noptions == 0;
		break;
	case 'j':
	case KEY_DOWN:
		if (selected < noptions - 1) {
			refreshed = 0;
			selected++;
		}
		break;
	}

	if (!refreshed)
		settings_draw();
}

void settings_draw(void) {
	int x, y;
	getmaxyx(settings.win, y, x);
	draw_border(settings.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	set_color(settings.win, selected == 0 ? &cs.texthl : &cs.text);
	for (int i = 0; i < noptions; i++) {
		set_color(settings.win, selected == i ? &cs.texthl : &cs.text);
		mvwprintw(settings.win, 1 + i, 2, "< %s >", options[i]);
	}

	wrefresh(settings.win);
	refreshed = 1;
}

void settings_resize(void) {
	wresize(settings.win, 35, 30);
	mvwin(settings.win, 7, 7);

	settings_draw();
}
