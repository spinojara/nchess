#ifndef UCIOPTIONS_H
#define UCIOPTIONS_H

#include <curses.h>

#include "engine.h"

void ucioptions_event(chtype ch, MEVENT *event);

void ucioptions_resize(void);

int ucioptions_init(const char *command, const char *workingdir, int nuo, const struct ucioption *uo);

#endif
