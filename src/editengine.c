#include "editengine.h"

#include <string.h>
#include <stdlib.h>

#include "window.h"
#include "color.h"
#include "draw.h"
#include "field.h"
#include "ucioptions.h"
#include "info.h"

struct field field[3];

int nucioption;
static struct ucioption *ucioption = NULL;
static struct uciengine *edit = NULL;

static int refreshed = 0;

static int selected = 0;

void editengine_draw(void);
void editengine_save(void);
void editengine_remove(void);
void ucioption_free(int *nuo, struct ucioption **uo);

void editengine_event(chtype ch, MEVENT *event) {
	refreshed = 0;
	switch (ch) {
	case 0:
		break;
	case KEY_ESC:
		refreshed = 1;
		ucioption_free(&nucioption, &ucioption);
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
		else if (event->y == 6 && 6 <= event->x && event->x < 21) {
			refreshed = 1;
			if (!ucioptions_init(field_buffer(&field[1], 0), field_buffer(&field[2], 0), nucioption, ucioption))
				place_top(&ucioptions);
			else
				info("Engine Error", "An error occured while parsing the UCI options.", INFO_ERROR, 6, 30);
		}
		break;
	case 'k':
		if (selected <= 2) {
			field_driver(&field[selected], ch, NULL);
			break;
		}
		/* fallthrough */
	case KEY_UP:
		selected = (selected + 5) % 6;
		break;
	case KEY_ENTER:
	case '\n':
		if (selected < 3)
			selected++;
		else if (selected == 3)
			editengine_save();
		else if (selected == 4)
			editengine_remove();
		else if (selected == 5) {
			refreshed = 1;
			switch (ucioptions_init(field_buffer(&field[1], 0), field_buffer(&field[2], 0), nucioption, ucioption)) {
			case 0:
				place_top(&ucioptions);
				break;
			case 1:
				info("Engine Error", "An error occured while parsing the UCI options.", INFO_ERROR, 6, 30);
				break;
			case 2:
				info("Engine Error", "The engine is unresponsive.", INFO_ERROR, 5, 33);
				break;
			default:
				break;
			}
		}
		break;
	case 'j':
		if (selected <= 2) {
			field_driver(&field[selected], ch, NULL);
			break;
		}
		/* fallthrough */
	case KEY_DOWN:
	case '\t':
		selected = (selected + 1) % 6;
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
	if (!field[0].len) {
		field[0].error = 1;
		return;
	}
	if (!field[1].len) {
		field[1].error = 1;
		return;
	}
	selected = 0;
	refreshed = 1;
	engines_add(edit, field_buffer(&field[0], 0), field_buffer(&field[1], 0), field_buffer(&field[2], 0), nucioption, ucioption);
	nucioption = 0;
	ucioption = NULL;
	for (int i = 0; i < 3; i++)
		field_clear(&field[i]);
	place_top(&engines);
}

void editengine_remove(void) {
	selected = 0;
	refreshed = 1;
	ucioption_free(&nucioption, &ucioption);
	if (edit)
		engines_remove(edit);
	place_top(&engines);
}

void editengine_draw(void) {
	int x, y;
	getmaxyx(editengine.win, y, x);
	draw_border(editengine.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	field_draw(&field[0], A_UNDERLINE, selected == 0, 0);
	field_draw(&field[1], A_UNDERLINE, selected == 1, 0);
	field_draw(&field[2], A_UNDERLINE, selected == 2, 0);
	set_color(editengine.win, &cs.text);
	mvwaddstr(editengine.win, 1, 2, "Name:");
	mvwaddstr(editengine.win, 2, 2, "Command:");
	mvwaddstr(editengine.win, 3, 2, "Working Dir:");
	set_color(editengine.win, selected == 3 ? &cs.texthl : &cs.text);
	mvwaddstr(editengine.win, 4, 10, "< Save >");
	set_color(editengine.win, selected == 4 ? &cs.texthl : &cs.text);
	mvwaddstr(editengine.win, 5, 9, edit ? "< Remove >" : "< Cancel >");
	set_color(editengine.win, selected == 5 ? &cs.texthl : &cs.text);
	mvwaddstr(editengine.win, 6, 6, "< UCI Options >");

	wrefresh(editengine.win);
	refreshed = 1;
}

void editengine_resize(void) {
	wresize(editengine.win, 35, 30);
	mvwin(editengine.win, 7, 7);

	editengine_draw();
}

int filtername(char c) {
	return c < 0x20 || c > 0x7E || c == '*';
}

int filterpath(char c) {
	return c < 0x20 || c > 0x7E;
}

void editengine_init(void) {
	field_init(&field[0], editengine.win, 1, 8, 18, &filtername, "bitbit");
	field_init(&field[1], editengine.win, 2, 11, 15, &filterpath, "bitbit");
	field_init(&field[2], editengine.win, 3, 15, 11, &filterpath, NULL);
}

void ucioption_free(int *nuo, struct ucioption **uo) {
	for (int i = 0; i < *nuo; i++) {
		free((*uo)[i].name);
		if ((*uo)[i].type == TYPE_STRING || (*uo)[i].type == TYPE_COMBO) {
			free((*uo)[i].value.str);
			free((*uo)[i].def.str);
		}
		if ((*uo)[i].type == TYPE_COMBO) {
			for (int j = 0; j < (*uo)[i].nvar; j++)
				free((*uo)[i].var[j]);
			free((*uo)[i].var);
		}
	}
	free(*uo);
	*uo = NULL;
	*nuo = 0;
}

struct ucioption *ucioption_copy(int nuo, const struct ucioption *src) {
	struct ucioption *dst = calloc(nuo, sizeof(*dst));
	for (int i = 0; i < nuo; i++) {
		dst[i].name = strdup(src[i].name);
		dst[i].type = src[i].type;
		switch (dst[i].type) {
		case TYPE_CHECK:
		case TYPE_SPIN:
			dst[i].value.i = src[i].value.i;
			break;
		case TYPE_STRING:
		case TYPE_COMBO:
			dst[i].value.str = strdup(src[i].value.str);
			break;
			break;
		case TYPE_BUTTON:
		default:
			break;
		}
	}
	return dst;
}

void editengine_uci(int nuo, struct ucioption *uo) {
	ucioption_free(&nucioption, &ucioption);
	nucioption = nuo;
	ucioption = uo;
}

void editengine_edit(struct uciengine *ue) {
	/* Save old edited engine in this case. */
	if (!ue && !edit)
		return;
	edit = ue;
	nucioption = 0;
	ucioption = NULL;
	field_clear(&field[0]);
	field_clear(&field[1]);
	field_clear(&field[2]);

	if (!edit)
		return;

	ucioption_free(&nucioption, &ucioption);
	nucioption = ue->nucioption;
	ucioption = ucioption_copy(ue->nucioption, ue->ucioption);

	for (unsigned i = 0; i < strlen(edit->name); i++)
		field_driver(&field[0], edit->name[i], NULL);
	for (unsigned i = 0; i < strlen(edit->command); i++)
		field_driver(&field[1], edit->command[i], NULL);
	for (unsigned i = 0; i < strlen(edit->workingdir); i++)
		field_driver(&field[2], edit->workingdir[i], NULL);
}
