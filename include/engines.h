#ifndef ENGINES_H
#define ENGINES_H

#include <ncurses.h>

enum {
	TYPE_CHECK,
	TYPE_SPIN,
	TYPE_STRING,
	TYPE_BUTTON,
};

/* value is a NULL pointer if there is no value,
 * i.e. for Clear Hash.
 */
struct ucioption {
	char *name;
	char *value;
	int type;
};

struct uciengine {
	char *name;
	char *command;
	char *workingdir;
	struct ucioption *ucioptions;
};

void engines_event(chtype ch, MEVENT *event);

void engines_resize(void);

#endif
