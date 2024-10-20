#ifndef DRAW_H
#define DRAW_H

#include <ncurses.h>

#include "color.h"

void draw_fill(WINDOW *win, struct color *bg, int ymin, int xmin, int ysize, int xsize, int (*exclude)(int, int));

void draw_border(WINDOW *win, struct color *bg, struct color *upper, struct color *lower, int fill, int ymin, int xmin, int ysize, int xsize);

#endif
