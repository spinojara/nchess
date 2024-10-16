#include "draw.h"

void draw_fill(WINDOW *win, struct color *bg, int ymin, int xmin, int ysize, int xsize) {
	wattrset(win, bg->attr);
	for (int y = ymin; y < ymin + ysize; y++)
		mvwhline(win, y, xmin, ' ', xsize);
}

void draw_border(WINDOW *win, struct color *bg, struct color *upper, struct color *lower, int fill, int ymin, int xmin, int ysize, int xsize) {
	if (fill)
		draw_fill(win, &cs.bordershadow, ymin, xmin, ysize, xsize);

	if (bg) {
		ysize -= 1;
		xsize -= 2;
	}

	int ymax = ysize + ymin - 1;
	int xmax = xsize + xmin - 1;

	wattrset(win, upper->attr);
	mvwhline(win, ymin, xmin, 0, xsize);
	mvwvline(win, ymin, xmin, 0, ysize);
	mvwaddch(win, ymin, xmin, ACS_ULCORNER);
	mvwaddch(win, ymax, xmin, ACS_LLCORNER);

	wattrset(win, lower->attr);
	mvwhline(win, ymax, xmin + 1, 0, xsize - 1);
	mvwvline(win, ymin, xmax, 0, ysize);
	mvwaddch(win, ymin, xmax, ACS_URCORNER);
	mvwaddch(win, ymax, xmax, ACS_LRCORNER);

	if (bg) {
		wattrset(win, cs.shadow.attr);
		mvwhline(win, ymax + 1, xmin + 1, ' ', xsize + 1);
		mvwvline(win, ymin + 1, xmax + 1, ' ', ysize - 1);
		mvwvline(win, ymin + 1, xmax + 2, ' ', ysize - 1);

		wattrset(win, bg->attr);
		mvwhline(win, ymax + 1, xmin, ' ', 2);
		mvwhline(win, ymin, xmax + 1, ' ', 2);
	}
}

void resize() {
}
