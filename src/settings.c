#include "settings.h"

#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#include "window.h"
#include "color.h"
#include "draw.h"
#include "mainwin.h"
#include "engines.h"

static int refreshed = 0;

static int selected = 0;

static int noconfig = 1;
static int configchanged = 0;

static const char *options[] = { "Flip Board", "Relative Scores", "Auto Flip", "Hide Engine Output" };

static int noptions = sizeof(options) / sizeof(*options);

void settings_draw();

void settings_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_MOUSE:
		if (1 <= event->y && event->y <= noptions && 2 <= event->x && event->x < 26) {
			refreshed = 0;
			selected = event->y - 1;
		}
		else {
			break;
		}
		/* fallthrough */
	case KEY_ENTER:
	case '\n':
	case ' ':
		switch (selected) {
		case 0:
			flipped = !flipped;
			break;
		case 1:
			relativescore = !relativescore;
			break;
		case 2:
			autoflip = !autoflip;
			break;
		case 3:
			hideengineoutput = !hideengineoutput;
			break;
		}
		configchanged = 1;
		mainwin.event(0, NULL);
		refreshed = 0;
		break;
	case KEY_ESC:
	case 'q':
		refreshed = 1;
		place_top(&topbar);
		break;
	case 'k':
	case KEY_UP:
		selected = (selected + noptions - 1) % noptions;
		refreshed = 0;
		break;
	case '\t':
	case 'j':
	case KEY_DOWN:
		selected = (selected + 1) % noptions;
		refreshed = 0;
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
		set_color(settings.win, &cs.text);
		if ((i == 0 && flipped) || (i == 1 && relativescore) || (i == 2 && autoflip) || (i == 3 && hideengineoutput))
			mvwaddch(settings.win, 1 + i, 2, '*');
		set_color(settings.win, selected == i ? &cs.texthl : &cs.text);
		mvwprintw(settings.win, 1 + i, 4, "< %s >", options[i]);
	}

	wrefresh(settings.win);
	refreshed = 1;
}

void settings_resize(void) {
	wresize(settings.win, 35, 30);
	mvwin(settings.win, 7, 7);

	settings_draw();
}

int settings_readconfig(void) {
#ifndef _WIN32
	const char *home = getenv("HOME");
	if (home == NULL)
		return 1;

	char buf[8192];
	if (strlen(home) > 4192)
		return 1;

	sprintf(buf, "%s/.config/nchess/nchess.conf", home);
	FILE *f = fopen(buf, "r");
	if (!f) {
		noconfig = 0;
		return 0;
	}

	char flippedstr[16];
	char relativescorestr[16];
	char autoflipstr[16];
	char hideengineoutputstr[16];
	if (!readconfig_option(f, "flipped=", flippedstr, sizeof(flippedstr)))
		goto error;
	if (!readconfig_option(f, "relativescore=", relativescorestr, sizeof(relativescorestr)))
		goto error;
	if (!readconfig_option(f, "autoflip=", autoflipstr, sizeof(autoflipstr)))
		goto error;
	if (!readconfig_option(f, "hideengineoutput=", hideengineoutputstr, sizeof(hideengineoutputstr)))
		goto error;

	flipped = flippedstr[0] == '1';
	relativescore = relativescorestr[0] == '1';
	autoflip = autoflipstr[0] == '1';
	hideengineoutput = hideengineoutputstr[0] == '1';

	fclose(f);
	noconfig = 0;
	return 0;

error:
	return 1;
#else
	return 0;
#endif
}

int settings_writeconfig(void) {
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

	strcat(buf, "/nchess.conf");
	FILE *f = fopen(buf, "w");
	if (!f)
		return 1;

	fprintf(f, "flipped=%d\n", flipped);
	fprintf(f, "relativescore=%d\n", relativescore);
	fprintf(f, "autoflip=%d\n", autoflip);
	fprintf(f, "hideengineoutput=%d\n", hideengineoutput);

	fclose(f);
#endif
	return 0;
}
