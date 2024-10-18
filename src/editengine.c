#include "editengine.h"

#include <string.h>

#include "window.h"
#include "color.h"
#include "draw.h"
#include "field.h"

struct field field[3];

static struct uciengine *edit = NULL;

static int refreshed = 0;

static int selected = 0;

void editengine_draw(void);
void editengine_save(void);
void editengine_remove(void);

void editengine_event(chtype ch, MEVENT *event) {
	refreshed = 0;
	switch (ch) {
	case 0:
		break;
	case KEY_ESC:
		refreshed = 1;
		place_top(&engines);
		break;
	case KEY_MOUSE:
		if (1 <= event->y && event->y <= 3) {
			selected = event->y - 1;
			if (selected <= 2)
				field_driver(&field[selected], ch, event);
		}
		else if (event->y == 4 && 10 <= event->x && event->x < 18)
				editengine_save();
		else if (event->y == 5 && 9 <= event->x && event->x < 19)
				editengine_remove();
		break;
	case KEY_UP:
		if (selected > 0)
			selected--;
		break;
	case KEY_DOWN:
		if (selected < 4)
			selected++;
		break;
	case KEY_ENTER:
	case '\n':
		if (selected < 3)
			selected++;
		else if (selected == 3)
			editengine_save();
		else if (selected == 4) {
			editengine_remove();
		}
		break;
	case '\t':
		selected = (selected + 1) % 4;
		break;
	default:
		if (selected <= 2)
			field_driver(&field[selected], ch, NULL);
	}

	if (!refreshed)
		editengine_draw();
}

void editengine_save(void) {
	/* name and command fields cannot be empty. */
	if (!field[0].len || !field[1].len)
		return;
	selected = 0;
	refreshed = 1;
	engines_add(edit, field_buffer(&field[0]), field_buffer(&field[1]), field_buffer(&field[2]));
	for (int i = 0; i < 3; i++)
		field_clear(&field[i]);
	place_top(&engines);
}

void editengine_remove(void) {
	selected = 0;
	refreshed = 1;
	engines_remove(edit);
	place_top(&engines);
}

void editengine_draw(void) {
	int x, y;
	getmaxyx(editengine.win, y, x);
	draw_border(editengine.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	field_draw(&field[0], A_UNDERLINE, selected == 0);
	field_draw(&field[1], A_UNDERLINE, selected == 1);
	field_draw(&field[2], A_UNDERLINE, selected == 2);
	set_color(editengine.win, &cs.text);
	mvwaddstr(editengine.win, 1, 2, "Name:");
	mvwaddstr(editengine.win, 2, 2, "Command:");
	mvwaddstr(editengine.win, 3, 2, "Working dir:");
	set_color(editengine.win, selected == 3 ? &cs.texthl : &cs.text);
	mvwaddstr(editengine.win, 4, 10, "< Save >");
	set_color(editengine.win, selected == 4 ? &cs.texthl : &cs.text);
	mvwaddstr(editengine.win, 5, 9, edit ? "< Remove >" : "< Cancel >");

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
	field_init(&field[1], editengine.win, 2, 11, 15, NULL);
	field_init(&field[2], editengine.win, 3, 15, 11, NULL);
}

void editengine_edit(struct uciengine *ue) {
	/* Save old edited engine in this case. */
	if (!ue && !edit)
		return;
	edit = ue;
	field_clear(&field[0]);
	field_clear(&field[1]);
	field_clear(&field[2]);

	if (!edit)
		return;

	for (unsigned i = 0; i < strlen(edit->name); i++)
		field_driver(&field[0], edit->name[i], NULL);
	for (unsigned i = 0; i < strlen(edit->command); i++)
		field_driver(&field[1], edit->command[i], NULL);
	for (unsigned i = 0; i < strlen(edit->workingdir); i++)
		field_driver(&field[2], edit->workingdir[i], NULL);
}
