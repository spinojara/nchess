#ifndef COLOR_H
#define COLOR_H

#include <ncurses.h>

struct color {
	chtype attr;

	int fg;
	int bg;
};

struct colors {
	struct color wpw;
	struct color bpw;
	struct color cpw;
	struct color wpb;
	struct color bpb;
	struct color cpb;
	struct color wps;
	struct color bps;
	struct color cps;

	struct color bg;
	struct color border;
	struct color bordershadow;
	struct color shadow;

	struct color text;
	struct color texthl;
	struct color textdim;
	struct color textblue;

	struct color red;
};

extern struct colors cs;

void set_color(WINDOW *win, struct color *c);

void init_colors(void);

#endif
