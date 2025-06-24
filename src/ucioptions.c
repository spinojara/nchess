#include "ucioptions.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#ifndef _WIN32
#include <poll.h>
#endif

#include "window.h"
#include "color.h"
#include "draw.h"
#include "field.h"
#include "engine.h"
#include "editengine.h"
#include "util.h"

#define MAXLINES 29

static int refreshed = 0;

static int selected = 0;

static int scrolloption = 0;
static int nucioption;
static struct ucioption *ucioption;
static struct field *ucioptionfield;

void ucioptions_draw(void);
void ucioptions_free_all(void);
void ucioptions_free_fields(void);
int ucioptions_fetch_fields(void);
void ucioptions_restore(void);

void ucioptions_event(chtype ch, MEVENT *event) {
#ifndef _WIN32
	refreshed = 0;
	switch (ch) {
	case 0:
		break;
	case KEY_ESC:
		refreshed = 1;
		ucioptions_free_all();
		place_top(&editengine);
		break;
	case KEY_MOUSE:
		if (1 <= event->y && event->y <= min(nucioption, MAXLINES + 1) && 2 <= event->x && event->x < 26) {
			selected = event->y - 1 + scrolloption;
			if (ucioption[selected].type == TYPE_STRING || ucioption[selected].type == TYPE_SPIN) {
				field_driver(&ucioptionfield[selected], ch, event);
				break;
			}
		}
		else if (event->y == min(nucioption + 1, MAXLINES + 2) && 9 <= event->x && event->x < 20) {
			selected = nucioption;
		}
		else if (event->y == min(nucioption + 2, MAXLINES + 3) && 10 <= event->x && event->x < 18) {
			selected = nucioption + 1;
		}
		else {
			break;
		}
		/* fallthrough */
	case '\n':
		if (selected == nucioption) {
			ucioptions_restore();
			break;
		}
		if (selected == nucioption + 1) {
			if (nucioption > 0) {
				if (ucioptions_fetch_fields())
					break;
				ucioptions_free_fields();
				editengine_uci(nucioption, ucioption);
				nucioption = 0;
				ucioption = NULL;
			}
			else {
				ucioptions_free_all();
			}
			refreshed = 1;
			place_top(&editengine);
			break;
		}
		switch (ucioption[selected].type) {
		case TYPE_STRING:
		case TYPE_SPIN:
			selected++;
			break;
		case TYPE_CHECK:
			ucioption[selected].value.i = !ucioption[selected].value.i;
			break;
		case TYPE_COMBO:;
			int i;
			for (i = 0; i < ucioption[selected].nvar; i++)
				if (!strcmp(ucioption[selected].var[i], ucioption[selected].value.str))
					break;
			assert(i < ucioption[selected].nvar);
			i = (i + 1) % ucioption[selected].nvar;
			free(ucioption[selected].value.str);
			ucioption[selected].value.str = strdup(ucioption[selected].var[i]);
			break;
		case TYPE_BUTTON:
			/* FIXME */
		default:
			break;
		}
		break;
	case 'k':
		if (selected < nucioption && ucioption[selected].type == TYPE_STRING) {
			field_driver(&ucioptionfield[selected], ch, NULL);
			break;
		}
		/* fallthrough */
	case KEY_UP:
		selected = (selected + nucioption + 1) % (nucioption + 2);
		if (selected > scrolloption + MAXLINES)
			scrolloption = min(selected, nucioption - 1) - MAXLINES;
		if (selected < scrolloption)
			scrolloption = selected;
		break;
	case 'j':
		if (selected < nucioption && ucioption[selected].type == TYPE_STRING) {
			field_driver(&ucioptionfield[selected], ch, NULL);
			break;
		}
		/* fallthrough */
	case KEY_DOWN:
	case '\t':
		selected = (selected + 1) % (nucioption + 2);
		if (selected > scrolloption + MAXLINES)
			scrolloption = min(selected, nucioption - 1) - MAXLINES;
		if (selected < scrolloption)
			scrolloption = selected;
		break;
	case ' ':
		switch (ucioption[selected].type) {
		case TYPE_COMBO:;
			int i;
			for (i = 0; i < ucioption[selected].nvar; i++)
				if (!strcmp(ucioption[selected].var[i], ucioption[selected].value.str))
					break;
			assert(i < ucioption[selected].nvar);
			i = (i + 1) % ucioption[selected].nvar;
			free(ucioption[selected].value.str);
			ucioption[selected].value.str = strdup(ucioption[selected].var[i]);
			break;
			break;
		case TYPE_BUTTON:
			/* FIXME press button */
			break;
		case TYPE_SPIN:
			ch = '+';
			break;
		case TYPE_CHECK:
			ucioption[selected].value.i = !ucioption[selected].value.i;
			break;
		default:
			break;
		}
		/* fallthrough */
	default:
		if (selected < nucioption) {
			switch (ucioption[selected].type) {
			case TYPE_STRING:
				field_driver(&ucioptionfield[selected], ch, NULL);
				break;
			case TYPE_SPIN:
				if (ch == '+') {
					char *endptr;
					errno = 0;
					ucioption[selected].value.i = strtoll(field_buffer(&ucioptionfield[selected], 0), &endptr, 10);
					if (errno || *endptr != '\0')
						/* i - 1 so that i++ makes it the default value. */
						ucioption[selected].value.i = ucioption[selected].def.i - 1;
					ucioption[selected].value.i++;
					if (ucioption[selected].value.i > ucioption[selected].max)
						ucioption[selected].value.i = ucioption[selected].min;
					char buf[BUFSIZ];
					sprintf(buf, "%ld", ucioption[selected].value.i);
					field_set(&ucioptionfield[selected], buf);
				}
				else if (ch == '-' && strlen(field_buffer(&ucioptionfield[selected], 0)) > 0) {
					char *endptr;
					errno = 0;
					ucioption[selected].value.i = strtoll(field_buffer(&ucioptionfield[selected], 0), &endptr, 10);
					if (errno || *endptr != '\0')
						ucioption[selected].value.i = ucioption[selected].def.i + 1;
					ucioption[selected].value.i--;
					if (ucioption[selected].value.i < ucioption[selected].min)
						ucioption[selected].value.i = ucioption[selected].max;
					char buf[BUFSIZ];
					sprintf(buf, "%ld", ucioption[selected].value.i);
					field_set(&ucioptionfield[selected], buf);
				}
				else {
					field_driver(&ucioptionfield[selected], ch, NULL);
				}
				break;
			default:
				break;
			}
		}
	}

	if (!refreshed)
		ucioptions_draw();
#else
	return;
#endif
}

#ifndef _WIN32
int filter_integer(char c) {
	return !(('0' <= c && c <= '9') || c == '-');
}
#endif

int ucioptions_init(const char *command, const char *workingdir, int nuo, const struct ucioption *uo) {
#ifndef _WIN32
	ucioptions_free_all();
	scrolloption = 0;
	selected = 0;
	char buf[BUFSIZ], *endptr;

	pid_t pid;
	int parentchild[2];
	int childparent[2];

	if (pipe(parentchild) || pipe(childparent))
		die("error: failed to open pipe");

	if ((pid = fork()) == -1)
		die("error: failed to fork");

	if (pid == 0) {
		close(parentchild[1]);
		close(childparent[0]);
		dup2(parentchild[0], STDIN_FILENO);
		dup2(childparent[1], STDOUT_FILENO);
		if (workingdir[0])
			if (chdir(workingdir))
				exit(-1);

		execlp(command, command, (char *)NULL);
		exit(-1);
	}

	close(parentchild[0]);
	close(childparent[1]);

	FILE *w = fdopen(parentchild[1], "w");
	if (!w)
		die("error: fdopen\n");
	FILE *r = fdopen(childparent[0], "r");
	if (!r)
		die("error: fdopen\n");

	fprintf(w, "uci\n");
	fprintf(w, "stop\n");
	fprintf(w, "quit\n");
	fflush(w);

	struct pollfd fd = { .fd = childparent[0], .events = POLLIN };

	int error = 0;
	int responsive = 0;
	while (!error) {
		if (poll(&fd, 1, 1000) <= 0 || !fgets(buf, sizeof(buf), r)) {
			responsive = -1;
			break;
		}

		if (!strncmp(buf, "id name ", 8)) {
			responsive++;
		}
		if (!strncmp(buf, "id author ", 10)) {
			responsive++;
		}
		if (!strcmp(buf, "uciok\n")) {
			responsive++;
			break;
		}
		if (strncmp(buf, "option ", 7))
			continue;
		nucioption++;
		ucioption = realloc(ucioption, nucioption * sizeof(*ucioption));
		if (!ucioption)
			die("error: realloc");
		ucioption[nucioption - 1].name = NULL;
		ucioption[nucioption - 1].value.str = NULL;
		ucioption[nucioption - 1].def.str = NULL;
		ucioptionfield = realloc(ucioptionfield, nucioption * sizeof(*ucioptionfield));
		ucioptionfield[nucioption - 1].init = 0;
		if (!ucioptionfield)
			die("error: realloc");

		char *name = strstr(buf, " name ");
		char *type = strstr(buf, " type ");
		char *def = strstr(buf, " default ");
		char *min1 = strstr(buf, " min ");
		char *max1 = strstr(buf, " max ");
		char *var = strstr(buf, " var ");

		if (!name) {
			error = UE_NONAME;
			break;
		}
		if (!type) {
			error = UE_NOTYPE;
			break;
		}

		int (*filter)(char) = NULL;

		if (strstr(buf, " type check")) {
			ucioption[nucioption - 1].type = TYPE_CHECK;
		}
		else if (strstr(buf, " type spin")) {
			ucioption[nucioption - 1].type = TYPE_SPIN;
			filter = &filter_integer;
		}
		else if (strstr(buf, " type string")) {
			ucioption[nucioption - 1].type = TYPE_STRING;
		}
		else if (strstr(buf, " type button")) {
			ucioption[nucioption - 1].type = TYPE_BUTTON;
		}
		else if (strstr(buf, " type combo")) {
			ucioption[nucioption - 1].type = TYPE_COMBO;
		}
		else {
			error = UE_BADTYPE;
			break;
		}

		ucioption[nucioption - 1].name = calloc(type - (name + 6) + 1, 1);
		strncpy(ucioption[nucioption - 1].name, name + 6, type - (name + 6));

		switch (ucioption[nucioption - 1].type) {
		case TYPE_CHECK:
			if (strstr(buf, " default true"))
				ucioption[nucioption - 1].def.i = 1;
			else
				ucioption[nucioption - 1].def.i = 0;
			break;
		case TYPE_SPIN:
			if (!min1) {
				ucioption[nucioption - 1].min = INT64_MIN;
			}
			else {
				errno = 0;
				ucioption[nucioption - 1].min = strtoll(min1 + 5, &endptr, 10);
				if (errno || (*endptr != '\n' && *endptr != ' ')) {
					error = UE_BADMIN;
					break;
				}
			}
			if (!max1) {
				ucioption[nucioption - 1].max = INT64_MAX;
			}
			else {
				errno = 0;
				ucioption[nucioption - 1].max = strtoll(max1 + 5, &endptr, 10);
				if (errno || (*endptr != '\n' && *endptr != ' ')) {
					error = UE_BADMAX;
					break;
				}
			}
			if (def) {
				errno = 0;
				ucioption[nucioption - 1].def.i = strtoll(def + 9, &endptr, 10);
				if (errno || (*endptr != '\n' && *endptr != ' ')) {
					error = UE_BADDEF;
					break;
				}
			}
			else {
				ucioption[nucioption - 1].def.i = min(max(ucioption[nucioption - 1].min, 0), ucioption[nucioption - 1].max);
			}
			break;
		case TYPE_COMBO:
			if (!var) {
				error = UE_NOVAR;
				break;
			}
			if (def) {
				int length = var - def - 9;
				if (length <= 0) {
					error = UE_BADDEF;
					break;
				}
				ucioption[nucioption - 1].def.str = calloc(length + 1, 1);
				strncat(ucioption[nucioption - 1].def.str, def + 9, length);
			}
			char *cur = buf;
			ucioption[nucioption - 1].nvar = 0;
			ucioption[nucioption - 1].var = NULL;
			while ((cur = strstr(cur, " var "))) {
				cur += 5;
				ucioption[nucioption - 1].nvar++;
				ucioption[nucioption - 1].var = realloc(ucioption[nucioption - 1].var, ucioption[nucioption - 1].nvar * sizeof(*(ucioption[nucioption - 1].var)));
				if (strstr(cur, " var ")) {
					ucioption[nucioption - 1].var[ucioption[nucioption - 1].nvar - 1] = calloc(strstr(cur, " var ") - cur + 1, 1);
					strncpy(ucioption[nucioption - 1].var[ucioption[nucioption - 1].nvar - 1], cur, strstr(cur, " var ") - cur);
				}
				else {
					ucioption[nucioption - 1].var[ucioption[nucioption - 1].nvar - 1] = calloc(strlen(cur), 1);
					memcpy(ucioption[nucioption - 1].var[ucioption[nucioption - 1].nvar - 1], cur, strlen(cur) - 1);
				}
			}
			if (ucioption[nucioption - 1].nvar == 0) {
				error = UE_NOVAR;
				break;
			}

			if (!ucioption[nucioption - 1].def.str)
				ucioption[nucioption - 1].def.str = strdup(ucioption[nucioption - 1].var[0]);

			int found = 0;
			for (int i = 0; i < ucioption[nucioption - 1].nvar; i++)
				if (!strcmp(ucioption[nucioption - 1].var[i], ucioption[nucioption - 1].def.str))
					found = 1;
			if (!found) {
				error = UE_DEFNOTVAR;
				break;
			}
			break;
		case TYPE_STRING:
			if (def) {
				int length = strlen(def + 9);
				if (length <= 0) {
					error = UE_BADDEF;
					break;
				}
				ucioption[nucioption - 1].def.str = calloc(length, 1);
				strncat(ucioption[nucioption - 1].def.str, def + 9, length - 1);
			}
			else {
				ucioption[nucioption - 1].def.str = strdup("");
			}
			break;
		default:
			break;
		}

		if (error)
			break;

		int len = strlen(ucioption[nucioption - 1].name);
		if (len > 15)
			len = 15;

		field_init(&ucioptionfield[nucioption - 1], ucioptions.win, nucioption, 4 + len, 22 - len, filter, NULL);

		switch (ucioption[nucioption - 1].type) {
		case TYPE_CHECK:
		case TYPE_SPIN:
			ucioption[nucioption - 1].value.i = ucioption[nucioption - 1].def.i;
			break;
		case TYPE_STRING:
		case TYPE_COMBO:
			ucioption[nucioption - 1].value.str = strdup(ucioption[nucioption - 1].def.str);
			break;
		}

		if (uo) {
			for (int i = 0; i < nuo; i++) {
				if (!strcmp(ucioption[nucioption - 1].name, uo[i].name) && ucioption[nucioption - 1].type == uo[i].type) {
					switch (uo[i].type) {
					case TYPE_STRING:
					case TYPE_COMBO:
						free(ucioption[nucioption - 1].value.str);
						ucioption[nucioption - 1].value.str = strdup(uo[i].value.str);
						break;
					case TYPE_SPIN:
					case TYPE_CHECK:
						ucioption[nucioption - 1].value.i = uo[i].value.i;
						break;
					default:
						break;
					}
				}
			}
		}

		switch (ucioption[nucioption - 1].type) {
		case TYPE_SPIN:
			sprintf(buf, "%ld", ucioption[nucioption - 1].value.i);
			field_set(&ucioptionfield[nucioption - 1], buf);
			break;
		case TYPE_STRING:
			field_set(&ucioptionfield[nucioption - 1], ucioption[nucioption - 1].value.str);
			break;
		default:
			break;
		}
	}

	kill(pid, SIGKILL);

	if (responsive != 3)
		error = UE_NORESPONSE;

	if (error)
		ucioptions_free_all();
	return error;
#else
	return 0;
#endif
}

#ifndef _WIN32
void ucioptions_free_fields(void) {
	for (int i = 0; i < nucioption; i++)
		field_free(&ucioptionfield[i]);
	free(ucioptionfield);
	ucioptionfield = NULL;
}

void ucioptions_free_all(void) {
	ucioptions_free_fields();
	ucioption_free(&nucioption, &ucioption);
}
#endif

void ucioptions_draw(void) {
#ifndef _WIN32
	int x, y;
	getmaxyx(ucioptions.win, y, x);
	draw_border(ucioptions.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);

	int start = scrolloption;
	int end = min(nucioption - 1, MAXLINES + start);

	for (int i = start; i <= end; i++) {
		char *name = strdup(ucioption[i].name);
		if (ucioption[i].type == TYPE_BUTTON) {
			if (strlen(name) > 24) {
				name[21] = '.';
				name[22] = '.';
				name[23] = '.';
				name[24] = '\0';
			}
			set_color(ucioptions.win, selected == i ? &cs.texthl : &cs.text);
			mvwaddstr(ucioptions.win, i + 1 - start, 2, name);
		}
		else if (ucioption[i].type == TYPE_CHECK) {
			set_color(ucioptions.win, selected == i ? &cs.texthl : &cs.text);
			if (strlen(name) > 19) {
				name[16] = '.';
				name[17] = '.';
				name[18] = '.';
				name[19] = '\0';
			}
			mvwaddstr(ucioptions.win, i + 1 - start, 2, name);
			mvwaddstr(ucioptions.win, i + 1 - start, 2 + strlen(name), ": [");
			mvwaddch(ucioptions.win, i + 1 - start, 6 + strlen(name), ']');
			mvwaddch(ucioptions.win, i + 1 - start, 5 + strlen(name), ucioption[i].value.i ? '*' : ' ');
		}
		else if (ucioption[i].type == TYPE_COMBO) {
			if (strlen(name) > 17) {
				name[14] = '.';
				name[15] = '.';
				name[16] = '.';
				name[17] = '\0';
			}
			set_color(ucioptions.win, selected == i ? &cs.texthl : &cs.text);
			mvwaddstr(ucioptions.win, i + 1 - start, 2, name);
			mvwaddstr(ucioptions.win, i + 1 - start, 2 + strlen(name), ": ");
			mvwaddnstr(ucioptions.win, i + 1 - start, 4 + strlen(name), ucioption[i].value.str, 5 + 17 - strlen(name));
		}
		else {
			if (strlen(name) > 15) {
				name[12] = '.';
				name[13] = '.';
				name[14] = '.';
				name[15] = '\0';
			}
			set_color(ucioptions.win, &cs.text);
			mvwaddstr(ucioptions.win, i + 1 - start, 2, name);
			mvwaddch(ucioptions.win, i + 1 - start, 2 + strlen(name), ':');
			ucioptionfield[i].y = i + 1 - start;
			field_draw(&ucioptionfield[i], A_UNDERLINE, selected == i, 0);
		}
		free(name);
	}
	set_color(ucioptions.win, &cs.text);
	set_color(ucioptions.win, selected == nucioption ? &cs.texthl : &cs.text);
	mvwaddstr(ucioptions.win, min(nucioption + 1, MAXLINES + 2), 9, "< Restore >");
	set_color(ucioptions.win, selected == nucioption  + 1 ? &cs.texthl : &cs.text);
	mvwaddstr(ucioptions.win, min(nucioption + 2, MAXLINES + 3), 10, "< Save >");

	wrefresh(ucioptions.win);
	refreshed = 1;
#endif
}

void ucioptions_resize(void) {
	wresize(ucioptions.win, 35, 30);
	mvwin(ucioptions.win, 7, 7);

	ucioptions_draw();
}

#ifndef _WIN32
int ucioptions_fetch_fields(void) {
	char *endptr;
	for (int i = 0; i < nucioption; i++) {
		switch (ucioption[i].type) {
		case TYPE_STRING:
			free(ucioption[i].value.str);
			ucioption[i].value.str = strdup(field_buffer(&ucioptionfield[i], 0));
			break;
		case TYPE_SPIN:
			ucioptionfield[i].error = 0;
			errno = 0;
			ucioption[i].value.i = strtoll(field_buffer(&ucioptionfield[i], 0), &endptr, 10);
			if (errno || *endptr != '\0') {
				ucioptionfield[i].error = 1;
				return 1;
			}
			if (ucioption[i].value.i > ucioption[i].max)
				ucioption[i].value.i = ucioption[i].max;
			if (ucioption[i].value.i < ucioption[i].min)
				ucioption[i].value.i = ucioption[i].min;
			break;
		default:
			break;
		}
	}
	return 0;
}

void ucioptions_restore(void) {
	for (int i = 0; i < nucioption; i++) {
		switch (ucioption[i].type) {
		case TYPE_COMBO:
		case TYPE_STRING:
			free(ucioption[i].value.str);
			ucioption[i].value.str = strdup(ucioption[i].def.str);
			field_set(&ucioptionfield[i], ucioption[i].value.str);
			break;
		case TYPE_CHECK:
		case TYPE_SPIN:;
			ucioption[i].value.i = ucioption[i].def.i;
			char buf[BUFSIZ];
			sprintf(buf, "%ld", ucioption[i].value.i);
			field_set(&ucioptionfield[i], buf);
			break;
		case TYPE_BUTTON:
			break;
		default:
			assert(0);
		}
	}
}
#endif
