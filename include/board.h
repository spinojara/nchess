#ifndef BOARD_H
#define BOARD_H

#include <curses.h>

#include "color.h"
#include "position.h"
#include "move.h"

void board_draw(WINDOW *win, int y, int x, struct position *pos, int selected, int flipped, struct move *bestmove);

void piece_draw(WINDOW *win, int y, int x, struct piece *p, struct color *fg);

#endif
