#include "color.h"

struct colors cs;

#define COLOR(c, f, b, a)   \
	do {                \
		c.fg = f;   \
		c.bg = b;   \
		c.attr = a; \
	} while(0)

void make_color(struct color *c) {
	static int pairs = 0;
	init_pair(++pairs, c->fg, c->bg);
	c->attr |= COLOR_PAIR(pairs);
}

void make_colors(void) {
	int has_dim = A_DIM != A_NORMAL;

	COLOR(cs.wpw, COLOR_WHITE, COLOR_CYAN, A_BOLD);
	COLOR(cs.bpw, COLOR_BLACK, COLOR_CYAN, A_NORMAL);
	COLOR(cs.cpw, COLOR_BLACK, COLOR_CYAN, A_BOLD);

	COLOR(cs.wpb, COLOR_WHITE, COLOR_BLUE, A_BOLD);
	COLOR(cs.bpb, COLOR_BLACK, COLOR_BLUE, A_NORMAL);
	COLOR(cs.cpb, COLOR_BLACK, COLOR_BLUE, A_BOLD);

	COLOR(cs.wps, COLOR_WHITE, COLOR_RED, A_BOLD);
	COLOR(cs.bps, COLOR_BLACK, COLOR_RED, A_NORMAL);
	COLOR(cs.cps, COLOR_BLACK, COLOR_RED, A_BOLD);

	COLOR(cs.wpm, COLOR_WHITE, COLOR_MAGENTA, A_BOLD);
	COLOR(cs.bpm, COLOR_BLACK, COLOR_MAGENTA, A_NORMAL);
	COLOR(cs.cpm, COLOR_BLACK, COLOR_MAGENTA, A_BOLD);

	COLOR(cs.bg,           COLOR_BLUE,   COLOR_BLUE,  A_NORMAL);
	COLOR(cs.border,       COLOR_WHITE,  COLOR_WHITE, A_BOLD);
	COLOR(cs.bordershadow, COLOR_BLACK,  COLOR_WHITE, A_NORMAL);
	COLOR(cs.shadow,       COLOR_BLACK,  COLOR_BLACK, A_NORMAL);

	COLOR(cs.text,         COLOR_BLACK,  COLOR_WHITE, A_NORMAL);
	COLOR(cs.texthl,       COLOR_WHITE,  COLOR_BLUE,  A_NORMAL | A_BOLD);
	COLOR(cs.textdim,      has_dim ? COLOR_WHITE : COLOR_BLACK,  COLOR_WHITE, A_DIM);
	COLOR(cs.textblue,     COLOR_BLUE,   COLOR_WHITE, A_NORMAL);
	COLOR(cs.texterror,    COLOR_RED,    COLOR_WHITE, A_NORMAL);

	COLOR(cs.red,          COLOR_BLACK,  COLOR_RED,   A_NORMAL);
	COLOR(cs.reddim,       COLOR_WHITE,  COLOR_RED,   A_DIM);

	make_color(&cs.wpw);
	make_color(&cs.bpw);
	make_color(&cs.cpw);

	make_color(&cs.wpb);
	make_color(&cs.bpb);
	make_color(&cs.cpb);

	make_color(&cs.wps);
	make_color(&cs.bps);
	make_color(&cs.cps);

	make_color(&cs.wpm);
	make_color(&cs.bpm);
	make_color(&cs.cpm);

	make_color(&cs.bg);
	make_color(&cs.border);
	make_color(&cs.bordershadow);
	make_color(&cs.shadow);

	make_color(&cs.text);
	make_color(&cs.texthl);
	make_color(&cs.textdim);
	make_color(&cs.textblue);
	make_color(&cs.texterror);

	make_color(&cs.red);
	make_color(&cs.reddim);
}

void set_color(WINDOW *win, struct color *c) {
	wattrset(win, c->attr);
}

void init_colors(void) {
	start_color();

	make_colors();
}
