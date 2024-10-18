#ifndef EDITENGINE_H
#define EDITENGINE_H

#include <ncurses.h>

#include "engines.h"

void editengine_event(chtype ch, MEVENT *event);

void editengine_resize(void);

void editengine_init(void);

void editengine_edit(struct uciengine *ue);

#endif
