#include "editwin.h"

#include "color.h"
#include "window.h"
#include "draw.h"
#include "position.h"
#include "board.h"
#include "mainwin.h"

static int refreshed = 0;

struct position pos;

static int selectedsquare = -1;

static int selectedpiece = -1;

static int illegal = 0;

void editwin_draw(void);

void editwin_event(chtype ch, MEVENT *event) {
	if (ch != 0 || ch != KEY_MOUSE || !(event->bstate & BUTTON1_RELEASED))
		illegal = 0;
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case KEY_UP:
		place_top(&topbar);
		break;
	case KEY_MOUSE:
		if (event->bstate & BUTTON1_PRESSED) {
			/* Board. */
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && selectedpiece == -1) {
				int new = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (flipped)
					new = 63 - new;
				if (pos.mailbox[new].type != EMPTY) {
					selectedsquare = new;
					selectedpiece = -1;
				}
				refreshed = 0;
			}
			else {
				selectedsquare = -1;
			}
			if (84 <= event->x && event->x < 104 && 0 < event->y && event->y < 30) {
				selectedsquare = -1;
				int new = (event->x - 84) / 10 + 2 * ((event->y - 1) / 5);
				selectedpiece = new == selectedpiece ? -1 : new;
				refreshed = 0;
			}
			if (event->y == 32 && 85 <= event->x && event->x < 89 + 5) {
				pos.turn = WHITE;
				refreshed = 0;
			}
			else if (event->y == 32 && 95 <= event->x && event->x < 99 + 5) {
				pos.turn = BLACK;
				refreshed = 0;
			}
			else if (event->y == 34 && 85 <= event->x && event->x < 89 + 5) {
				pos.K = !pos.K;
				refreshed = 0;
			}
			else if (event->y == 34 && 95 <= event->x && event->x < 99 + 5) {
				pos.k = !pos.k;
				refreshed = 0;
			}
			else if (event->y == 36 && 85 <= event->x && event->x < 89 + 5) {
				pos.Q = !pos.Q;
				refreshed = 0;
			}
			else if (event->y == 36 && 95 <= event->x && event->x < 99 + 5) {
				pos.q = !pos.q;
				refreshed = 0;
			}
			else if (event->y == 43 && 85 <= event->x && event->x < 85 + 8) {
				if (make_legal(&pos)) {
					refreshed = 1;
					set_position(&pos);
					place_top(&mainwin);
				}
				else {
					illegal = 1;
					refreshed = 0;
				}
			}
			else if (event->y == 43 && 85 + 11 <= event->x && event->x < 85 + 11 + 8) {
				refreshed = 1;
				place_top(&mainwin);
			}
		}
		if (event->bstate & BUTTON1_RELEASED) {
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && selectedsquare != -1) {
				int to = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (flipped)
					to = 63 - to;
				if (to == selectedsquare)
					break;
				pos.mailbox[to] = pos.mailbox[selectedsquare];
				pos.mailbox[selectedsquare].type = EMPTY;
				selectedsquare = -1;
				refreshed = 0;
			}
			else if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && selectedpiece != -1) {
				int to = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (flipped)
					to = 63 - to;
				pos.mailbox[to].type = PAWN + selectedpiece / 2;
				pos.mailbox[to].color = (selectedpiece + 1) % 2;
				/* The same piece should still be selected so we do not reset it. */
				refreshed = 0;
			}
			else if (selectedsquare != -1) {
				pos.mailbox[selectedsquare].type = EMPTY;
				selectedsquare = -1;
				refreshed = 0;
			}
		}
		break;
	}

	if (!refreshed)
		editwin_draw();
}

void editwin_draw(void) {
	draw_fill(editwin.win, &cs.border, 0, 0, LINES, COLS);
	draw_border(editwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 0, 5 * 8 + 2, 10 * 8 + 2);
	draw_border(editwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 0, 3, 82);
	draw_border(editwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 83, 42, 24);
	draw_border(editwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 83, 3, 24);
	struct piece p;
	for (int color = BLACK; color <= WHITE; color++) {
		for (int type = PAWN; type <= KING; type++) {
			p.type = type;
			p.color = color;
			piece_draw(editwin.win, 1 + 5 * (type - PAWN), 84 + 10 * (1 - color), &p, selectedpiece != - 1 && (selectedpiece + 1) % 2 == color && type == PAWN + selectedpiece / 2 ? color == WHITE ? &cs.wpb : &cs.bpb : color == WHITE ? &cs.border : &cs.text);
		}
	}
	set_color(editwin.win, pos.turn ? &cs.texthl : &cs.text);
	mvwaddstr(editwin.win, 32, 85, "< White >");
	set_color(editwin.win, pos.turn ? &cs.text : &cs.texthl);
	mvwaddstr(editwin.win, 32, 95, "< Black >");
	set_color(editwin.win, pos.K ? &cs.texthl : &cs.text);
	mvwaddstr(editwin.win, 34, 86, "< O-O >");
	set_color(editwin.win, pos.Q ? &cs.texthl : &cs.text);
	mvwaddstr(editwin.win, 36, 85, "< O-O-O >");
	set_color(editwin.win, pos.k ? &cs.texthl : &cs.text);
	mvwaddstr(editwin.win, 34, 96, "< O-O >");
	set_color(editwin.win, pos.q ? &cs.texthl : &cs.text);
	mvwaddstr(editwin.win, 36, 95, "< O-O-O >");
	set_color(editwin.win, &cs.text);
	char str[3];
	mvwprintw(editwin.win, 38, 87, "En Passant: %s", algebraic(str, pos.en_passant ? pos.en_passant : -1));
	mvwaddstr(editwin.win, 43, 85 + 11, "< Back >");
	set_color(editwin.win, illegal ? &cs.red : &cs.text);
	mvwaddstr(editwin.win, 43, 85, "< Save >");

	board_draw(editwin.win, 1, 1, &pos, selectedsquare, flipped);
	fen_draw(editwin.win, &pos);
	wrefresh(editwin.win);
	refreshed = 1;
}

void editwin_resize(void) {
	wresize(editwin.win, LINES - 8, COLS - 10);
	mvwin(editwin.win, 5, 4);

	editwin_draw();
}
