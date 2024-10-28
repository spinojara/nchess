#include <locale.h>
#include <time.h>
#include <signal.h>

#include <curses.h>

#include "position.h"
#include "board.h"
#include "color.h"
#include "move.h"
#include "draw.h"
#include "window.h"
#include "mainwin.h"
#include "editengine.h"
#include "info.h"
#include "editwin.h"
#include "newgame.h"

int running = 1;

int main(void) {
	signal(SIGPIPE, SIG_IGN);
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
	newgame_init();
	window_resize();
	editengine_init();

	if (engines_readconfig()) {
		running = 0;
		info("Config Error", "An unrecoverable error occured while parsing the engine configuration file (~/.config/nchess/engine.conf). Please fix or delete the file, then try again.", INFO_ERROR, 7, 62);
	}

	MEVENT event;
	int ch;
	while (running) {
		ch = wgetch(wins[0]->win);
		update_game();
		if (ch == ERR) {
			ch = 0;
			if (wins[0] != &mainwin)
				mainwin.event(0, NULL);
		}
		if (ch == KEY_MOUSE) {
			if (getmouse(&event) != OK)
				continue;
			if (event.bstate & BUTTON1_RELEASED && ((wins[0] != &mainwin && wins[0] != &editwin) || !wenclose(mainwin.win, event.y, event.x)))
				continue;
			if (event.bstate & BUTTON1_RELEASED) {
				/* This is fine even if editwin is on top, the windows are the same size. */
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
	end_analysis();
	end_game();
	engines_writeconfig();
}
