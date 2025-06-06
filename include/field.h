#ifndef FIELD_H
#define FIELD_H

#include <curses.h>

struct field {
	int screenlen;

	int len;
	int size;
	/* Careful, str is not always null terminated. */
	char *str;
	char *suggestion;

	int cur;
	int disp;

	int (*filter)(char);

	WINDOW *win;
	int y, x;

	int error;
	int init;
};

void field_init(struct field *field, WINDOW *win, int y, int x, int screenlen, int (*filter)(char), const char *suggestion);

void field_draw(struct field *field, attr_t attr, int draw_cursor, int blocket);

void field_driver(struct field *field, chtype ch, MEVENT *event);

const char *field_buffer(struct field *field, int use_suggestion);

void field_clear(struct field *field);

void field_set(struct field *field, const char *str);

void field_insert(struct field *field, char c);

void field_free(struct field *field);

#endif
