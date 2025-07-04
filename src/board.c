#include "board.h"

#include <assert.h>

#include "position.h"
#include "color.h"

char pawn_image[5][10] = {
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', ' ', 'o', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', '/', ' ','\\', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', '(', 'F', 'F', 'F', ')', ' ', ' ' },
};

char knight_image[5][10] = {
	{ ' ', ' ', ' ', ' ', '_', '-', '/', ' ', ' ', ' ' },
	{ ' ', ' ', '/', ' ', 'o', ' ', ' ', ')', ' ', ' ' },
	{ ' ', ' ', ' ', '-', '-', '/', ' ', ' ', ')', ' ' },
	{ ' ', ' ', ' ', '/', ' ', ' ', ' ', ' ', '|', ' ' },
	{ ' ', ' ', '(', 'F', 'F', 'F', 'F', 'F', ')', ' ' },
};

char bishop_image[5][10] = {
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', ' ', 'o', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', '(','\\', ')', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', '/', ' ','\\', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', '(', 'F', 'F', 'F', ')', ' ', ' ' },
};

char rook_image[5][10] = {
	{ ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', '|', '-', '|', '-', '|', ' ', ' ' },
	{ ' ', ' ', ' ', '[', ' ', ' ', ' ', ']', ' ', ' ' },
	{ ' ', ' ', ' ', '|', ' ', ' ', ' ', '|', ' ', ' ' },
	{ ' ', ' ', ' ', '(', 'F', 'F', 'F', ')', ' ', ' ' },
};

char queen_image[5][10] = {
	{ ' ', ' ', ' ', ' ', '_', 'o', '_', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ','\\', ' ', '/', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', '(', ' ', ')', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', '/', ' ', ' ', ' ','\\', ' ', ' ' },
	{ ' ', ' ', '(', 'F', 'F', 'F', 'F', 'F', ')', ' ' },
};

char king_image[5][10] = {
	{ ' ', ' ', ' ', ' ', '_', '+', '_', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ','\\', ' ', '/', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', ' ', '(', ' ', ')', ' ', ' ', ' ' },
	{ ' ', ' ', ' ', '/', ' ', ' ', ' ','\\', ' ', ' ' },
	{ ' ', ' ', '(', 'F', 'F', 'F', 'F', 'F', ')', ' ' },
};

void piece_draw(WINDOW *win, int y, int x, struct piece *p, struct color *fg) {
	char (*image)[10] = NULL;
	int i, j;
	switch (p->type) {
	case EMPTY:
		break;
	case PAWN:
		image = pawn_image;
		break;
	case KNIGHT:
		image = knight_image;
		break;
	case BISHOP:
		image = bishop_image;
		break;
	case ROOK:
		image = rook_image;
		break;
	case QUEEN:
		image = queen_image;
		break;
	case KING:
		image = king_image;
		break;
	default:
		assert(0);
		return;
	}

	set_color(win, fg);
	for (i = 0; i < 5; i++) {
		for (j = 0; j < 10; j++) {
			chtype c = p->type != EMPTY ? image[i][j] : ' ';
			if (c == 'F') {
				c = ACS_BLOCK;
			}
			mvwaddch(win, y + i, x + j, c);
		}
	}
}

void board_draw(WINDOW *win, int y, int x, struct position *pos, int selected, int flipped, struct move *bestmove) {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			int sq = 8 * i + j;
			if (flipped)
				sq = 63 - sq;
			struct piece *p = &pos->mailbox[sq];
			struct color *fg;
			int is_selected = sq == selected;
			int is_move = !is_selected && bestmove && (sq == bestmove->from || sq == bestmove->to);
			if ((i + j) % 2) {
				if (p->color) {
					fg = &cs.wpw;
				}
				else {
					fg = &cs.bpw;
				}
			}
			else {
				if (p->color) {
					fg = &cs.wpb;
				}
				else {
					fg = &cs.bpb;
				}
			}
			if (is_selected) {
				if (fg == &cs.wpw || fg == &cs.wpb)
					fg = &cs.wps;
				if (fg == &cs.bpw || fg == &cs.bpb)
					fg = &cs.bps;
			}
			else if (is_move) {
				if (fg == &cs.wpw || fg == &cs.wpb)
					fg = &cs.wpm;
				if (fg == &cs.bpw || fg == &cs.bpb)
					fg = &cs.bpm;
			}
			piece_draw(win, y + 5 * (7 - i), x + 10 * j, p, fg);
			if (is_selected) {
				fg = &cs.cps;
			}
			else if (is_move) {
				fg = &cs.cpm;
			}
			else if ((i + j) % 2) {
				fg = &cs.cpw;
			}
			else {
				fg = &cs.cpb;
			}
			if (i == 0) {
				set_color(win, fg);
				mvwaddch(win, y + 5 * 7 + 4, x + 10 * j, 'a' + (flipped ? 7 - j : j));
			}
			if (j == 7) {
				set_color(win, fg);
				mvwaddch(win, y + 5 * (7 - i), x + 79, '1' + (flipped ? 7 - i : i));
			}
		}
	}
}


