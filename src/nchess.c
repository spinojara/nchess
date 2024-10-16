#include <locale.h>
#include <time.h>

#include <ncurses.h>

#include "position.h"
#include "board.h"
#include "color.h"
#include "move.h"
#include "draw.h"
#include "window.h"
#include "mainwin.h"
#include "editengine.h"

int main(void) {
	setlocale(LC_ALL, "");
	initscr();
	if (!has_colors()) {
		endwin();
		fprintf(stderr, "error: the terminal does not support color\n");
		return 1;
	}
	init_colors();
	keypad(stdscr, TRUE);
	set_escdelay(25);
	curs_set(0);
	cbreak();
	noecho();
	mouseinterval(0);
	mousemask(BUTTON1_PRESSED | BUTTON1_RELEASED, NULL);

	window_init();
	mainwin_init();
	window_resize();
	editengine_init();

	MEVENT event;
	chtype ch;
	while (1) {
		ch = wgetch(wins[0]->win);
		if (ch == 'q')
			break;
		else if (ch == KEY_MOUSE) {
			if (getmouse(&event) != OK)
				continue;
			if (event.bstate & BUTTON1_RELEASED && (wins[0] != &mainwin || !wenclose(mainwin.win, event.y, event.x)))
				continue;
			if (event.bstate & BUTTON1_RELEASED) {
				wmouse_trafo(mainwin.win, &event.y, &event.x, FALSE);
			}
			else if (event.bstate & BUTTON1_PRESSED) {
				int i;
				for (i = 0; i < nwins; i++) {
					if (wenclose(wins[i]->win, event.y, event.x)) {
						wmouse_trafo(wins[i]->win, &event.y, &event.x, FALSE);
						place_top(wins[i]);
						break;
					}
				}
				if (i == nwins)
					continue;
			}
			else
				continue;
		}
		else if (ch == KEY_RESIZE) {
			window_resize();
			continue;
		}
		wins[0]->event(ch, ch == KEY_MOUSE ? &event : NULL);
	}

	endwin();
}
