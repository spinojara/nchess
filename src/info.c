#include "info.h"

#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "color.h"
#include "draw.h"

int next_word(char *word, const char *text, int *i) {
	int j, flag;
	for (j = flag = 0; text[*i] != '\0'; ++*i) {
		if (text[*i] == ' ') {
			if (!flag)
				continue;
			else
				break;
		}
		flag = 1;
		word[j++] = text[*i];
	}
	word[j] = '\0';
	return j;
}

void info(const char *title, const char *message, int type, int lines, int cols) {
	WINDOW *win = newwin(lines, cols, 26 - lines / 2, 45 - cols / 2);
	keypad(win, TRUE);

	draw_border(win, NULL, &cs.border, &cs.bordershadow, 1, 0, 0, lines, cols);

	set_color(win, type == INFO_ERROR ? &cs.texterror : &cs.text);
	mvwaddch(win, 0, 1, ' ');
	mvwaddch(win, 0, strlen(title) + 2, ' ');
	mvwaddstr(win, 0, 2, title);
	set_color(win, &cs.text);

	int len = strlen(message);
	char *buf = malloc(len + 1);
	int line = 2;

	int i = 0, wordlen, col = 3;
	while (i < len) {
		 wordlen = next_word(buf, message, &i);
		 if (wordlen + col <= cols - 3) {
		 	mvwaddstr(win, line, col, buf);
		 }
		 else {
			line++;
			col = 3;
		 	mvwaddstr(win, line, col, buf);
		 }
		col += wordlen + 1;
	}

	free(buf);

	int ch;
	MEVENT event;
	if ((ch = wgetch(win)) != '\n' && ch != 'q')
		while (wgetch(win) == KEY_MOUSE && getmouse(&event) == OK && (event.bstate & BUTTON1_RELEASED));

	delwin(win);
}
