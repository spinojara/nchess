#include "field.h"

#include <stdlib.h>
#include <string.h>

#include "color.h"

void field_init(struct field *field, WINDOW *win, int y, int x, int screenlen, int (*filter)(char), const char *suggestion) {
	field->screenlen = screenlen;
	field->filter = filter;

	field->win = win;
	field->y = y;
	field->x = x;

	field->win = win;
	field->y = y;
	field->x = x;

	if (suggestion) {
		field->suggestion = malloc(strlen(suggestion) + 1);
		strcpy(field->suggestion, suggestion);
	}
	else {
		field->suggestion = NULL;
	}

	field->cur = field->disp = 0;
	field->len = 0;
	field->size = 32;
	field->str = malloc(field->size);
}

/* Draw the \n and \t characters as a red 'n' and 't' respectively. */
void field_draw(struct field *field, attr_t attr, int draw_cursor, int blocked) {
	for (int j = 0; j < field->screenlen; j++) {
		chtype c;
		if (field->len == 0 && field->suggestion && j < (int)strlen(field->suggestion))
			c = field->suggestion[j];
		else if (field->disp + j < field->len)
			c = field->str[field->disp + j];
		else
			c = ' ';

		wattrset(field->win, (draw_cursor && field->disp + j == field->cur ? cs.texthl.attr : field->len ? cs.text.attr : cs.textdim.attr) | attr);

		if (blocked) {
			c = ACS_HLINE | A_UNDERLINE;
			set_color(field->win, &cs.textdim);
		}

		switch (c) {
		case '\0':
			c = '0';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\v':
			c = 'v';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\n':
			c = 'n';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\t':
			c = 't';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\r':
			c = 'r';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\b':
			c = 'b';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\f':
			c = 'f';
			wattrset(field->win, cs.red.attr | attr);
			break;
		case '\a':
			c = 'a';
			wattrset(field->win, cs.red.attr | attr);
			break;
		}
		mvwaddch(field->win, field->y, field->x + j, c);
	}
}

void field_curinc(struct field *field) {
	if (field->cur == field->len)
		return;
	field->cur++;
	if (field->disp + field->screenlen <= field->cur)
		field->disp++;
}

void field_curdec(struct field *field) {
	if (field->cur == 0)
		return;
	field->cur--;
	if (field->disp > field->cur)
		field->disp--;
}

void field_insert(struct field *field, char c) {
	if (field->len >= field->size - 1) {
		field->size *= 2;
		field->str = realloc(field->str, field->size);
	}
	for (int i = field->len; i >= field->cur; i--)
		field->str[i + 1] = field->str[i];
	field->str[field->cur] = c;
	field->len++;

	field_curinc(field);
}

char *field_buffer(struct field *field) {
	field->str[field->len] = '\0';
	return field->str;
}

void field_backspace(struct field *field) {
	if (field->cur == 0)
		return;

	for (int i = field->cur; i <= field->len; i++)
		field->str[i - 1] = field->str[i];
	field_curdec(field);
	field->len--;
	/* Move cursor on backspace even if curmm did not. */
	if (0 < field->disp && field->disp < field->cur && field->disp + field->screenlen >= field->len)
		field->disp--;
}

void field_driver(struct field *field, chtype ch, MEVENT *event) {
	switch (ch) {
	case 127:
	case KEY_BACKSPACE:
		field_backspace(field);
		break;
	case KEY_LEFT:
		field_curdec(field);
		break;
	case KEY_RIGHT:
		field_curinc(field);
		break;
	case KEY_MOUSE:
		if (event->y == field->y && field->x <= event->x && event->x < field->x + field->screenlen) {
			field->cur = field->disp + event->x - field->x;
			if (field->cur > field->len)
				field->cur = field->len;
		}
		break;
	default:
		if (!field->filter || !field->filter(ch & A_CHARTEXT)) {
			field_insert(field, ch & A_CHARTEXT);
		}
	}
}

void field_clear(struct field *field) {
	field->len = 0;
	field->disp = 0;
	field->cur = 0;
}

void field_set(struct field *field, char *str) {
	field_clear(field);
	int len = strlen(str);
	for (int i = 0; i < len; i++)
		field_insert(field, str[i]);
}
