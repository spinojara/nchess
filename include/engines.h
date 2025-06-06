#ifndef ENGINES_H
#define ENGINES_H

#include <curses.h>

#include "engine.h"

extern int nengines;
extern int sengines;
extern struct uciengine *uciengines;

void engines_event(chtype ch, MEVENT *event);

void engines_resize(void);

void engines_add(struct uciengine *edit, const char *name, const char *command, const char *workingdir, int nucioption, struct ucioption *ucioption);

void engines_remove(struct uciengine *edit);

int engines_readconfig(void);

int engines_writeconfig(void);

char *readconfig_option(FILE *f, const char *prefix, char *s, size_t size);

#endif
