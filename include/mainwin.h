#ifndef MAINWIN_H
#define MAINWIN_H

#include <ncurses.h>

#include "engine.h"
#include "position.h"

extern struct position posd;

extern int flipped;

void mainwin_event(chtype ch, MEVENT *event);

void mainwin_resize(void);

void mainwin_init(void);

void start_analysis(struct uciengine *ue);

void end_analysis(void);

void fen_draw(WINDOW *win, struct position *pos);

void set_position(struct position *pos);

#endif
