#ifndef ENGINEPICKER_H
#define ENGINEPICKER_H

#include <ncurses.h>

#include "engine.h"

void enginepicker_event(chtype ch, MEVENT *event);

void enginepicker_resize(void);

void enginepicker_setup(const struct uciengine **e);

#endif
