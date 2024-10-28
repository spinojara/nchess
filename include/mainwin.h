#ifndef MAINWIN_H
#define MAINWIN_H

#include <curses.h>

#include "engine.h"
#include "position.h"

extern struct position posd;

extern int flipped;
extern int relativescore;
extern int gamerunning;
extern int autoflip;
extern int hideengineoutput;

void mainwin_event(chtype ch, MEVENT *event);

void mainwin_resize(void);

void mainwin_init(void);

void start_analysis(struct uciengine *ue);

void end_analysis(void);

void fen_draw(WINDOW *win, struct position *pos);

void set_position(const struct position *pos);

int fen_filter(char c);

void start_game(const struct uciengine *black, const struct uciengine *white, const struct position *start, const struct timecontrol timecontrol[2]);

void end_game(void);

char *position_fen(char *line, int displayed);

#endif
