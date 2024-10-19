#ifndef INFO_H
#define INFO_H

enum {
	INFO_MESSAGE,
	INFO_WARNING,
	INFO_ERROR,
};

void info(const char *title, const char *message, int type, int lines, int cols);

#endif
