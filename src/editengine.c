#include "editengine.h"

#include "window.h"
#include "color.h"
#include "draw.h"
#include "field.h"

struct field field[3];

static int refreshed = 0;

static int selected = 0;

void editengine_draw(void);

void editengine_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		break;
	case KEY_MOUSE:
		if (1 <= event->y && event->y <= 3) {
			selected = event->y - 1;
			field_driver(&field[selected], ch, event);
		}
		break;
	case KEY_UP:
		if (selected > 0)
			selected--;
		break;
	case KEY_DOWN:
	case KEY_ENTER:
	case '\n':
		if (selected < 2)
			selected++;
		break;
	case '\t':
		selected = (selected + 1) % 3;
		break;
	default:
		field_driver(&field[selected], ch, NULL);
	}
	refreshed = 0;

#if 1
	if (!refreshed)
		editengine_draw();
#endif
}

void editengine_draw(void) {
	int x, y;
	getmaxyx(editengine.win, y, x);
	draw_border(editengine.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	field_draw(&field[0], A_UNDERLINE, selected == 0);
	field_draw(&field[1], A_UNDERLINE, selected == 1);
	field_draw(&field[2], A_UNDERLINE, selected == 2);
	set_color(editengine.win, &cs.text);
	mvwprintw(editengine.win, 1, 2, "Name:");
	mvwprintw(editengine.win, 2, 2, "Working dir:");
	mvwprintw(editengine.win, 3, 2, "Command:");

	wrefresh(editengine.win);
	refreshed = 1;
}

void editengine_resize(void) {
	wresize(editengine.win, 35, 30);
	mvwin(editengine.win, 7, 7);

	editengine_draw();
}

void editengine_init(void) {
	field_init(&field[0], editengine.win, 1, 8, 18, NULL);
	field_init(&field[1], editengine.win, 2, 15, 11, NULL);
	field_init(&field[2], editengine.win, 3, 11, 15, NULL);
}
