#ifndef UCIOPTIONS_H
#define UCIOPTIONS_H

#include <curses.h>

#include "engine.h"

enum {
	UE_NOERROR,
	UE_NONAME,
	UE_NOTYPE,
	UE_BADTYPE,
	UE_BADMIN,
	UE_BADMAX,
	UE_BADDEF,
	UE_NOVAR,
	UE_DEFNOTVAR,
	UE_NORESPONSE,
};

void ucioptions_event(chtype ch, MEVENT *event);

void ucioptions_resize(void);

int ucioptions_init(const char *command, const char *workingdir, int nuo, const struct ucioption *uo);

#endif
