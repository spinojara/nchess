#include "mainwin.h"

#include <stdlib.h>
#include <string.h>

#include "window.h"
#include "draw.h"
#include "color.h"
#include "position.h"
#include "move.h"
#include "board.h"

static int refreshed = 0;

static int selectedsquare = -1;

/* Displayed position. */
static struct position posd;
/* Actual position. */
static struct position posa;

static int smove = 0;
static int selectedmove = -1;
static int nmove = 0;

static struct {
	struct move move;
	char name[8];
	int color;
	int fullmove;
} *vmove = NULL;

void mainwin_draw(void);
void put_move(struct move *move, int at_end);
int backward_move(void);
void backward_full(void);
int forward_move(void);
void forward_full(void);

void mainwin_event(chtype ch, MEVENT *event) {
	if (ch != KEY_MOUSE && ch != 0)
		selectedsquare = -1;
	switch (ch) {
	case 0:
		refreshed = 0;
		break;
	case 'u':
		backward_move();
		refreshed = 0;
		break;
	case 'f':
		forward_move();
		refreshed = 0;
		break;
	case 'k':
	case KEY_UP:
		place_top(&topbar);
		break;
	case KEY_MOUSE:
		if (event->bstate & BUTTON1_PRESSED) {
			/* Board. */
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41) {
				int new = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (posd.mailbox[new].type != EMPTY && posd.mailbox[new].color == posd.turn)
					selectedsquare = new;
				refreshed = 0;
			}
			/* Forward and backward. */
			else if (event->y == 5 * 8 + 3 && 86 <= event->x && event->x <= 89) {
				backward_full();
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 91 <= event->x && event->x <= 93) {
				backward_move();
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 96 <= event->x && event->x <= 98) {
				forward_move();
				refreshed = 0;
			}
			else if (event->y == 5 * 8 + 3 && 100 <= event->x && event->x <= 103) {
				forward_full();
				refreshed = 0;
			}
			else if (1 <= event->y && event->y <= 40 && 90 <= event->x && event->x != 97 && event->x <= 105 && nmove > 0) {
				int line = event->y - 1;
				int index = event->x > 96;
				int moveindex = 2 * line + index - (vmove[0].color == BLACK);
				if (0 <= moveindex && moveindex < nmove) {
					backward_full();
					for (int i = 0; i < moveindex + 1; i++)
						forward_move();
				}
				refreshed = 0;
			}
		}
		if (event->bstate & BUTTON1_RELEASED) {
			if (0 < event->x && event->x < 81 && 0 < event->y && event->y < 41 && selectedsquare != -1) {
				struct move move;
				int to = (event->x - 1) / 10 + 8 * (7 - (event->y - 1) / 5);
				if (to == selectedsquare)
					break;
				new_move(&move, selectedsquare, to, 0, 0);
				struct move moves[MOVES_MAX];
				movegen(&posd, moves, 0);
				for (int i = 0; !is_null(&moves[i]); i++) {
					if (moves[i].to == move.to && moves[i].from == move.from) {
						put_move(&moves[i], 0);
						break;
					}
				}
				selectedsquare = -1;
				refreshed = 0;
			}
		}
		break;
	}

	if (!refreshed)
		mainwin_draw();
}

int backward_move(void) {
	if (selectedmove == -1)
		return 1;
	undo_move(&posd, &vmove[selectedmove--].move);
	return 0;
}

int forward_move(void) {
	if (selectedmove > nmove - 2)
		return 1;
	do_move(&posd, &vmove[++selectedmove].move);
	return 0;
}

void backward_full(void) {
	while (!backward_move());
}

void forward_full(void) {
	while (!forward_move());
}

void put_move(struct move *move, int at_end) {
	int save_history = 0;
	if (!at_end) {
		if (nmove > selectedmove + 1) {
			/* Only delete history if we made a different move. */
			if (!movecmp(move, &vmove[selectedmove + 1].move))
				save_history = nmove;
			nmove = selectedmove + 1;
			posa = posd;
		}
	}
	if (nmove >= smove) {
		smove = smove ? 2 * smove : 4;
		vmove = realloc(vmove, smove * sizeof(*vmove));
	}

	vmove[nmove].move = *move;
	vmove[nmove].color = posa.turn;
	vmove[nmove].fullmove = posa.fullmove;
	move_pgn(vmove[nmove].name, &posa, move);
	do_move(&posa, move);

	nmove++;

	if (nmove == selectedmove + 2) {
		selectedmove++;
		do_move(&posd, move);
	}

	if (save_history)
		nmove = save_history;
}

void moves_draw(void) {
	if (smove == 0)
		return;
	set_color(mainwin.win, &cs.text);
	int offset = vmove[0].color == BLACK;
	int current = -offset;
	for (int line = 0; line < 40; line++, current += 2) {
		set_color(mainwin.win, &cs.text);
		mvwprintw(mainwin.win, 1 + line, 85, "                  ");
		if (current == -1 && nmove > 0) {
			mvwprintw(mainwin.win, 1 + line, 85, "%3d. ...", vmove[0].fullmove <= 999 ? vmove[0].fullmove : 999);
		}
		else if (current < nmove) {
			set_color(mainwin.win, &cs.text);
			mvwprintw(mainwin.win, 1 + line, 85, "%3d.", vmove[current].fullmove <= 999 ? vmove[current].fullmove : 999);
			set_color(mainwin.win, current == selectedmove ? &cs.texthl : &cs.text);
			mvwprintw(mainwin.win, 1 + line, 85 + 5, "%s", vmove[current].name);
		}
		if (current + 1 < nmove) {
			set_color(mainwin.win, current + 1 == selectedmove ? &cs.texthl : &cs.text);
			mvwprintw(mainwin.win, 1 + line, 85 + 13, "%s", vmove[current + 1].name);
		}
	}
}

void fen_draw(void) {
	char fen[128] = { 0 };
	pos_to_fen(fen, &posd);
	/* Limit the length of the fen. In very rare cases this would actually
	 * make parts of the fen invisible.
	 */
	fen[77] = '.';
	fen[78] = '.';
	fen[79] = '.';
	fen[80] = '\0';
	set_color(mainwin.win, &cs.text);
	mvwprintw(mainwin.win, 43, 1, "%s", fen);
}

void mainwin_draw(void) {
	draw_fill(mainwin.win, &cs.border, 0, 0, LINES, COLS);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 0, 5 * 8 + 2, 10 * 8 + 2);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 0, 3, 82);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 0, 83, 42, 24);
	draw_border(mainwin.win, NULL, &cs.bordershadow, &cs.border, 0, 5 * 8 + 2, 83, 3, 24);
	set_color(mainwin.win, &cs.text);
	mvwaddch(mainwin.win, 5 * 8 + 3, 87, ACS_LARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 88, ACS_LARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 92, ACS_LARROW);
#if 0
	mvwaddch(mainwin.win, 5 * 8 + 3, 92, ACS_VLINE);
	mvwaddch(mainwin.win, 5 * 8 + 3, 93, ACS_VLINE);
#endif
	mvwaddch(mainwin.win, 5 * 8 + 3, 97, ACS_RARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 101, ACS_RARROW);
	mvwaddch(mainwin.win, 5 * 8 + 3, 102, ACS_RARROW);

	board_draw(mainwin.win, 1, 1, &posd, selectedsquare);
	moves_draw();
	fen_draw();
	wrefresh(mainwin.win);
	refreshed = 1;
}

void mainwin_resize(void) {
	wresize(mainwin.win, LINES - 8, COLS - 10);
	mvwin(mainwin.win, 5, 4);

	mainwin_draw();
}

void mainwin_init(void) {
	pos_from_fen(&posa, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	posd = posa;
}
