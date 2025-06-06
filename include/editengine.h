#ifndef EDITENGINE_H
#define EDITENGINE_H

#include <curses.h>

#include "engines.h"

void editengine_event(chtype ch, MEVENT *event);

void editengine_resize(void);

void editengine_init(void);

void editengine_edit(struct uciengine *ue);

void editengine_uci(int nucioption, struct ucioption *ucioption);

void ucioption_free(int *nuo, struct ucioption **uo);

#endif
