#ifndef BOARD_H
#define BOARD_H

#include <ncurses.h>

#include "color.h"
#include "position.h"

void board_draw(WINDOW *win, int y, int x, struct position *pos, int selected);

void piece_draw(WINDOW *win, int y, int x, struct piece *p, struct color *fg);

#endif