#ifndef FIELD_H
#define FIELD_H

#include <ncurses.h>

struct field {
	int screenlen;

	int len;
	int size;
	/* Careful, str is not always null terminated. */
	char *str;

	int cur;
	int disp;

	int (*filter)(char);

	WINDOW *win;
	int y, x;
};

void field_init(struct field *field, WINDOW *win, int y, int x, int screenlen, int (*filter)(char));

void field_draw(struct field *field, attr_t attr, int draw_cursor);

void field_driver(struct field *field, chtype ch, MEVENT *event);

char *field_buffer(struct field *field);

void field_clear(struct field *field);

void field_set(struct field *field, char *str);

void field_insert(struct field *field, char c);

#endif
