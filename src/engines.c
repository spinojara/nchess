#include "engines.h"

#include <string.h>
#include <stdlib.h>

#include "window.h"
#include "color.h"
#include "draw.h"
#include "editengine.h"

static int refreshed = 0;

static int selected = 0;

static int nengines = 0; static int sengines = 0;
static struct uciengine *uciengines = NULL;

void engines_draw();

void engines_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_MOUSE:
		if (1 == event->y) {
			if (7 <= event->x && event->x < 21) {
				refreshed = 1;
				selected = 0;
				editengine_edit(NULL);
				place_top(&editengine);
			}
		}
		else if (2 <= event->y && event->y <= nengines + 1) {
			if (2 <= event->x && event->x < 26) {
				refreshed = 1;
				selected = event->y - 1;
				editengine_edit(&uciengines[selected - 1]);
				place_top(&editengine);
			}
		}
		break;
	case KEY_ENTER:
	case '\n':
		refreshed = 1;
		if (selected)
			editengine_edit(&uciengines[selected - 1]);
		else
			editengine_edit(NULL);
		place_top(&editengine);
		break;
	case KEY_ESC:
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
		selected = (selected + 1) % (1 + nengines);
		refreshed = nengines == 0;
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
		engines_draw();
}

void engines_add(struct uciengine *edit, char *name, char *command, char *workingdir) {
	char *namep = malloc(strlen(name) + 1);
	memcpy(namep, name, strlen(name) + 1);
	char *commandp = malloc(strlen(command) + 1);
	memcpy(commandp, command, strlen(command) + 1);
	char *workingdirp = malloc(strlen(workingdir) + 1);
	memcpy(workingdirp, workingdir, strlen(workingdir) + 1);
	if (edit) {
		free(edit->name);
		free(edit->command);
		free(edit->workingdir);
		edit->name = namep;
		edit->command = commandp;
		edit->workingdir = workingdirp;
		return;
	}

	if (nengines >= sengines) {
		sengines = sengines ? 2 * sengines : 4;
		uciengines = realloc(uciengines, sengines * sizeof(*uciengines));
	}
	uciengines[nengines].name = namep;
	uciengines[nengines].command = commandp;
	uciengines[nengines].workingdir = workingdirp;
	nengines++;
}

void engines_remove(struct uciengine *edit) {
	int i;
	for (i = 0; i < nengines; i++)
		if (edit == &uciengines[i])
			break;

	if (i == nengines)
		return;

	free(edit->name);
	free(edit->command);
	free(edit->workingdir);

	if (nengines)
		uciengines[i] = uciengines[--nengines];

	if (selected > nengines)
		selected = nengines;
}

void engines_draw(void) {
	int x, y;
	getmaxyx(engines.win, y, x);
	draw_border(engines.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	set_color(engines.win, selected == 0 ? &cs.texthl : &cs.text);
	mvwprintw(engines.win, 1, 7, "< New Engine >");
	for (int i = 0; i < nengines; i++) {
		set_color(engines.win, selected == 1 + i ? &cs.texthl : &cs.text);
		mvwaddstr(engines.win, 2 + i, 2, uciengines[i].name);
	}

	wrefresh(engines.win);
	refreshed = 1;
}

void engines_resize(void) {
	wresize(engines.win, 35, 30);
	mvwin(engines.win, 7, 7);

	engines_draw();
}
