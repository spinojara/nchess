#include "engines.h"

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "window.h"
#include "color.h"
#include "draw.h"
#include "editengine.h"

static int refreshed = 0;

static int selected = 0;

static int noconfig = 1;
static int configchanged = 0;

int nengines = 0;
int sengines = 0;
struct uciengine *uciengines = NULL;

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
	case ' ':
		refreshed = 1;
		if (selected)
			editengine_edit(&uciengines[selected - 1]);
		else
			editengine_edit(NULL);
		place_top(&editengine);
		break;
	case KEY_ESC:
	case 'q':
		place_top(&topbar);
		break;
	case 'k':
	case KEY_UP:
		selected = (selected + nengines) % (1 + nengines);
		refreshed = nengines == 0;
		break;
	case '\t':
	case 'j':
	case KEY_DOWN:
		selected = (selected + 1) % (1 + nengines);
		refreshed = nengines == 0;
		break;
	}

	if (!refreshed)
		engines_draw();
}

int enginecmp(const void *e1, const void *e2) {
	return strcmp(((struct uciengine *)e1)->name, ((struct uciengine *)e2)->name);
}

void engines_add(struct uciengine *edit, const char *name, const char *command, const char *workingdir, int nucioption, struct ucioption *ucioption) {
	if (!noconfig)
		configchanged = 1;
	char *namep = strdup(name);
	if (!namep)
		die("error: strdup\n");
	char *commandp = strdup(command);
	if (!commandp)
		die("error: strdup\n");
	char *workingdirp = strdup(workingdir);
	if (!workingdirp)
		die("error: strdup\n");
	if (edit) {
		free(edit->name);
		free(edit->command);
		free(edit->workingdir);
		ucioption_free(&edit->nucioption, &edit->ucioption);

		edit->name = namep;
		edit->command = commandp;
		edit->workingdir = workingdirp;
		edit->nucioption = nucioption;
		edit->ucioption = ucioption;
		goto sort;
	}

	if (nengines >= sengines) {
		sengines = sengines ? 2 * sengines : 4;
		uciengines = realloc(uciengines, sengines * sizeof(*uciengines));
		if (!uciengines)
			die("error: realloc\n");
	}
	uciengines[nengines].name = namep;
	uciengines[nengines].command = commandp;
	uciengines[nengines].workingdir = workingdirp;
	uciengines[nengines].nucioption = nucioption;
	uciengines[nengines].ucioption = ucioption;
	nengines++;
	selected = 0;

sort:
	qsort(uciengines, nengines, sizeof(*uciengines), &enginecmp);
}

void engines_remove(struct uciengine *edit) {
	if (!noconfig)
		configchanged = 1;
	int i;
	for (i = 0; i < nengines; i++)
		if (edit == &uciengines[i])
			break;

	if (i == nengines)
		return;

	free(edit->name);
	free(edit->command);
	free(edit->workingdir);
	ucioption_free(&edit->nucioption, &edit->ucioption);

	if (nengines)
		uciengines[i] = uciengines[--nengines];

	if (selected > nengines)
		selected = nengines;

	qsort(uciengines, nengines, sizeof(*uciengines), &enginecmp);
}

void engines_draw(void) {
	int x, y;
	getmaxyx(engines.win, y, x);
	draw_border(engines.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);
	set_color(engines.win, selected == 0 ? &cs.texthl : &cs.text);
	mvwprintw(engines.win, 1, 7, "< New Engine >");
	char name[24];
	for (int i = 0; i < nengines; i++) {
		set_color(engines.win, selected == 1 + i ? &cs.texthl : &cs.text);
		snprintf(name, 24, "%s", uciengines[i].name);
		if (strlen(name) == 23) {
			name[19] = '.';
			name[20] = '.';
			name[21] = '.';
			name[22] = '\0';
		}
		mvwaddstr(engines.win, 2 + i, 4, name);
	}

	wrefresh(engines.win);
	refreshed = 1;
}

void engines_resize(void) {
	wresize(engines.win, 35, 30);
	mvwin(engines.win, 7, 7);

	engines_draw();
}

char *readconfig_option(FILE *f, const char *prefix, char *s, size_t size) {
	char buf[8192];
	if (!fgets(buf, sizeof(buf), f))
		return NULL;

	if (strncmp(buf, prefix, strlen(prefix)))
		return NULL;

	if (s) {
		if (strlen(buf) - strlen(prefix) > size)
			return NULL;
	}

	if (!strchr(buf, '\n'))
		return NULL;
	*strchr(buf, '\n') = '\0';

	if (s)
		strcpy(s, buf + strlen(prefix));
	else
		s = strdup(buf + strlen(prefix));
	return s;
}

int engines_readconfig(void) {
#ifndef _WIN32
	const char *home = getenv("HOME");
	if (home == NULL)
		return 1;

	char buf[8192];
	if (strlen(home) > 4192)
		return 1;

	sprintf(buf, "%s/.config/nchess/engines.conf", home);
	FILE *f = fopen(buf, "r");
	if (!f) {
		noconfig = 0;
		return 0;
	}

	char *endptr;
	char name[8192];
	char command[8192];
	char workingdir[8192];
	char optionsstr[8192];
	int options;
	int loops = 0;
	while (1) {
		loops++;
		if (!fgets(buf, sizeof(buf), f))
			break;
		if (strcmp(buf, "[engine]\n"))
			goto error;

		if (!readconfig_option(f, "name=", name, sizeof(name)))
			goto error;
		if (!readconfig_option(f, "command=", command, sizeof(command)))
			goto error;
		if (!readconfig_option(f, "workingdir=", workingdir, sizeof(workingdir)))
			goto error;
		if (!readconfig_option(f, "options=", optionsstr, sizeof(optionsstr)))
			goto error;

		errno = 0;
		options = strtol(optionsstr, &endptr, 10);
		if (errno || *endptr != '\0' || options < 0)
			goto error;

		/* We don't have to free this memory in case of
		 * error since nchess exits anyway.
		 */
		struct ucioption *uo = NULL;
		if (options) {
			uo = calloc(options, sizeof(*uo));
			for (int i = 0; i < options; i++) {
				if (!(uo[i].name = readconfig_option(f, "name=", NULL, 0)))
					goto error;

				char type[2];
				char value[8192];
				if (!readconfig_option(f, "type=", type, sizeof(type)))
					goto error;

				uo[i].type = type[0] - '0';
				if (uo[i].type != TYPE_BUTTON && !readconfig_option(f, "value=", value, sizeof(value)))
					goto error;

				switch (uo[i].type) {
				case TYPE_CHECK:
				case TYPE_SPIN:
					errno = 0;
					uo[i].value.i = strtol(value, &endptr, 10);
					if (errno || *endptr != '\0')
						goto error;
					break;
				case TYPE_COMBO:
				case TYPE_STRING:
					uo[i].value.str = strdup(value);
					if (!uo[i].value.str)
						goto error;
					break;
				case TYPE_BUTTON:
					break;
				default:
					goto error;
				}
			}
		}

		engines_add(NULL, name, command, workingdir, options, uo);
	}

	fclose(f);
	noconfig = 0;
	return 0;

error:
	return 1;
#else
	return 0;
#endif
}

int engines_writeconfig(void) {
#ifndef _WIN32
	if (noconfig || !configchanged)
		return 0;
	const char *home = getenv("HOME");
	if (home == NULL)
		return 1;

	char buf[8192];
	if (strlen(home) > 4192)
		return 1;

	sprintf(buf, "%s/.config", home);
	errno = 0;
	if (mkdir(buf, 0700) && errno != EEXIST)
		return 1;

	strcat(buf, "/nchess");
	errno = 0;
	if (mkdir(buf, 0700) && errno != EEXIST)
		return 1;

	strcat(buf, "/engines.conf");
	FILE *f = fopen(buf, "w");
	if (!f)
		return 1;

	for (int i = 0; i < nengines; i++) {
		struct uciengine *e = &uciengines[i];
		fprintf(f, "[engine]\n");
		fprintf(f, "name=%s\n", e->name);
		fprintf(f, "command=%s\n", e->command);
		fprintf(f, "workingdir=%s\n", e->workingdir);
		fprintf(f, "options=%d\n", e->nucioption);
		for (int j = 0; j < e->nucioption; j++) {
			fprintf(f, "name=%s\n", e->ucioption[j].name);
			fprintf(f, "type=%d\n", e->ucioption[j].type);
			switch (e->ucioption[j].type) {
			case TYPE_CHECK:
			case TYPE_SPIN:
				fprintf(f, "value=%ld\n", e->ucioption[j].value.i);
				break;
			case TYPE_COMBO:
			case TYPE_STRING:
				fprintf(f, "value=%s\n", e->ucioption[j].value.str);
			case TYPE_BUTTON:
				break;
			}
		}
	}

	fclose(f);
#endif
	return 0;
}
