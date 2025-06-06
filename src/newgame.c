#include "newgame.h"

#include <string.h>
#include <stdlib.h>

#include "window.h"
#include "draw.h"
#include "color.h"
#include "field.h"
#include "enginepicker.h"
#include "mainwin.h"
#include "timepoint.h"
#include "info.h"

static int refreshed = 0;

static int currentposition = 0;

static int locktimecontrol = 1;
static struct field timecontrol[2];
static struct field fen;

static const struct uciengine *whiteplayer = NULL, *blackplayer = NULL;

static int selected = 0;

void newgame_draw(void);

void newgame_event(chtype ch, MEVENT *event) {
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_MOUSE:
		if (event->y == 3 && 2 <= event->x && event->x < 17) {
			selected = 0;
#ifndef _WIN32
			enginepicker_setup(&whiteplayer);
			place_top(&enginepicker);
			refreshed = 1;
#else
			not_supported();
			refreshed = 0;
#endif
		}
		else if (event->y == 3 && 19 <= event->x && event->x < 34) {
			selected = 1;
#ifndef _WIN32
			enginepicker_setup(&blackplayer);
			place_top(&enginepicker);
			refreshed = 1;
#else
			not_supported();
			refreshed = 0;
#endif
		}
		else if (event->y == 6 && 14 <= event->x && event->x < 14 + 8) {
			selected = 2;
			locktimecontrol = !locktimecontrol;
			refreshed = 0;
		}
		else if (event->y == 7 && 2 <= event->x && event->x < 17) {
			selected = 3;
			field_driver(&timecontrol[0], ch, event);
			refreshed = 0;
		}
		else if (event->y == 7 && 19 <= event->x && event->x < 34) {
			selected = 4;
			field_driver(&timecontrol[1], ch, event);
			refreshed = 0;
		}
		else if (event->y == 10 && 8 <= event->x && event->x < 28) {
			selected = 5;
			currentposition = !currentposition;
			refreshed = 0;
		}
		else if (event->y == 11 && 2 <= event->x && event->x < 34 && !currentposition) {
			selected = 6;
			field_driver(&fen, ch, event);
			refreshed = 0;
		}
		else if (event->y == 13 && 11 <= event->x && event->x < 25) {
			selected = 7;
			refreshed = 0;
			struct timecontrol tc[2];
			if (!timecontrol_string(&tc[WHITE], field_buffer(&timecontrol[0], 1)))
				break;
			if (!timecontrol_string(&tc[BLACK], field_buffer(&timecontrol[1], 1)))
				break;
			if (!currentposition && !fen_is_ok(field_buffer(&fen, 1)))
				break;
			if ((tc[WHITE].infinite && whiteplayer) || (tc[BLACK].infinite && blackplayer))
				break;
			refreshed = 1;
			struct position pos;
			start_game(blackplayer, whiteplayer, currentposition ? &posd : pos_from_fen(&pos, field_buffer(&fen, 1)), tc);
			place_top(&mainwin);
		}
		break;
	case 'q':
	case KEY_ESC:
		place_top(&topbar);
		refreshed = 1;
		break;
	case 'k':
		if (selected == 6) {
			field_driver(&fen, ch, NULL);
			refreshed = 0;
			break;
		}
		/* fallthrough */
	case KEY_UP:
		selected = (selected + 7) % 8;
		refreshed = 0;
		if (selected == 6 && currentposition)
			selected = 5;
		break;
	case 'h':
		if (selected == 1 || selected == 4) {
			selected--;
			refreshed = 0;
		}
		break;
	case 'l':
		if (selected == 0 || selected == 3) {
			selected++;
			refreshed = 0;
		}
		break;
	case 'j':
	case KEY_DOWN:
	case '\t':
		selected = (selected + 1) % 8;
		if (selected == 6 && currentposition)
			selected = 7;
		refreshed = 0;
		break;
	case KEY_ENTER:
	case '\n':
	case ' ':
		switch (selected) {
		case 0:
#ifndef _WIN32
			enginepicker_setup(&whiteplayer);
			place_top(&enginepicker);
			refreshed = 1;
#else
			not_supported();
			refreshed = 0;
#endif
			break;
		case 1:
#ifndef _WIN32
			enginepicker_setup(&blackplayer);
			place_top(&enginepicker);
			refreshed = 1;
#else
			not_supported();
			refreshed = 0;
#endif
			break;
		case 2:
			locktimecontrol = !locktimecontrol;
			if (locktimecontrol)
				field_set(&timecontrol[1], field_buffer(&timecontrol[0], 0));
			refreshed = 0;
			break;
		case 5:
			currentposition = !currentposition;
			refreshed = 0;
			break;
		case 7:;
			struct timecontrol tc[2];
			refreshed = 0;
			if (!timecontrol_string(&tc[WHITE], field_buffer(&timecontrol[0], 1))) {
				timecontrol[0].error = 1;
				break;
			}
			if (!timecontrol_string(&tc[BLACK], field_buffer(&timecontrol[1], 1))) {
				timecontrol[1].error = 1;
				break;
			}
			if (!currentposition && !fen_is_ok(field_buffer(&fen, 1))) {
				fen.error = 1;
				break;
			}
			if (tc[WHITE].infinite && whiteplayer) {
				timecontrol[0].error = 1;
				break;
			}
			if (tc[BLACK].infinite && blackplayer) {
				timecontrol[1].error = 1;
				break;
			}
			refreshed = 1;
			struct position pos;
			start_game(blackplayer, whiteplayer, currentposition ? &posd : pos_from_fen(&pos, field_buffer(&fen, 1)), tc);
			place_top(&mainwin);
			break;
		}
		if (ch != ' ')
			break;
		/* fallthrough */
	default:
		switch (selected) {
		case 0:
			if (ch == KEY_RIGHT) {
				selected = 1;
				refreshed = 0;
			}
			break;
		case 1:
			if (ch == KEY_LEFT) {
				selected = 0;
				refreshed = 0;
			}
			break;
		case 3:
		case 4:;
			int index = selected - 3;
			int other = 1 - index;
			field_driver(&timecontrol[index], ch, event);
			if (locktimecontrol) {
				field_set(&timecontrol[other], field_buffer(&timecontrol[index], 0));
				timecontrol[other].cur = timecontrol[index].cur;
				timecontrol[other].disp = timecontrol[index].disp;
			}
			refreshed = 0;
			break;
		case 6:
			if (!currentposition) {
				field_driver(&fen, ch, event);
				refreshed = 0;
			}
			break;
		}
		break;
	}

	if (!refreshed)
		newgame_draw();
}

void newgame_draw(void) {
	int x, y;
	getmaxyx(newgame.win, y, x);
	draw_border(newgame.win, &cs.bg, &cs.border, &cs.bordershadow, 1, 0, 0, y, x);

	wattrset(newgame.win, cs.text.attr | A_UNDERLINE);
	mvwaddstr(newgame.win, 2, 7, "White");
	mvwaddstr(newgame.win, 2, 24, "Black");
	mvwaddstr(newgame.win, 5, 12, "Time Control");
	set_color(newgame.win, &cs.text);
	set_color(newgame.win, selected == 0 ? &cs.texthl : &cs.text);
	if (!whiteplayer) {
		mvwaddstr(newgame.win, 3, 5, "< Human >");
	}
	else {
		char name[17];
		snprintf(name, 17, "%s", whiteplayer->name);
		if (strlen(name) == 16) {
			name[12] = '.';
			name[13] = '.';
			name[14] = '.';
			name[15] = '\0';
		}
		mvwaddstr(newgame.win, 3, (19 - strlen(name)) / 2, name);
	}
	set_color(newgame.win, selected == 1 ? &cs.texthl : &cs.text);
	if (!blackplayer) {
		mvwaddstr(newgame.win, 3, 22, "< Human >");
	}
	else {
		char name[17];
		snprintf(name, 17, "%s", blackplayer->name);
		if (strlen(name) == 16) {
			name[12] = '.';
			name[13] = '.';
			name[14] = '.';
			name[15] = '\0';
		}
		mvwaddstr(newgame.win, 3, (53 - strlen(name)) / 2, name);
	}
	if (locktimecontrol) {
		set_color(newgame.win, &cs.text);
		mvwaddch(newgame.win, 6, 12, '*');
	}
	set_color(newgame.win, selected == 2 ? &cs.texthl : &cs.text);
	mvwaddstr(newgame.win, 6, 14, "< Lock >");

	field_draw(&timecontrol[0], A_UNDERLINE, selected == 3, 0);
	field_draw(&timecontrol[1], A_UNDERLINE, selected == 4, 0);

	wattrset(newgame.win, cs.text.attr | A_UNDERLINE);
	mvwaddstr(newgame.win, 9, 14, "Opening");
	if (currentposition) {
		set_color(newgame.win, &cs.text);
		mvwaddch(newgame.win, 10, 6, '*');
	}
	set_color(newgame.win, selected == 5 ? &cs.texthl : &cs.text);
	mvwaddstr(newgame.win, 10, 8, "< Current Position >");
	field_draw(&fen, A_UNDERLINE, selected == 6, currentposition);

	set_color(newgame.win, selected == 7 ? &cs.texthl : &cs.text);
	mvwaddstr(newgame.win, 13, 11, "< Start Game >");

	refreshed = 1;
	wrefresh(newgame.win);
}

void newgame_resize(void) {
	wresize(newgame.win, 35, 38);
	mvwin(newgame.win, 7, 7);

	newgame_draw();
}

int filter_number(char c) {
	/* tc is given in the format 40/1:30+2 */
	return c != '.' && (c < '0' || c > '9') && c != '/' && c != ':' && c != '+' && c != 'i' && c != 'n' && c != 'f';
}

void newgame_init(void) {
	field_init(&timecontrol[0], newgame.win, 7, 2, 15, &filter_number, "40/3:00+2");
	field_init(&timecontrol[1], newgame.win, 7, 19, 15, &filter_number, "40/3:00+2");
	field_init(&fen, newgame.win, 11, 2, 32, &fen_filter, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}
