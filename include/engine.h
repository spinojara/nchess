#ifndef ENGINE_H
#define ENGINE_H

#include <sys/types.h>
#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#endif

#include "timepoint.h"

enum {
	TYPE_CHECK,
	TYPE_SPIN,
	TYPE_COMBO,
	TYPE_BUTTON,
	TYPE_STRING,
};

enum {
	EE_NONE,
	EE_TERMINATED,
	EE_CRASHED,
	EE_READYOK,
	EE_ILLEGALMOVE,
};

/* value is a NULL pointer if there is no value,
 * i.e. for Clear Hash.
 */
struct ucioption {
	char *name;
	int type;
	union {
		char *str;
		int64_t i;
	} value;
	union {
		char *str;
		int64_t i;
	} def;
	int64_t min;
	int64_t max;
	char **var;
};

struct uciengine {
	char *name;
	char *command;
	char *workingdir;
	int nucioption;
	struct ucioption *ucioption;
};

struct engineconnection {
	pid_t pid;
#ifndef _WIN32
	pthread_t tid;
#endif
	FILE *w, *r;

	timepoint_t isready;
	timepoint_t readyok;
	int error;

	char bestmove[128];
	char name[17];
	timepoint_t bestmovetime;
#ifndef _WIN32
	pthread_mutex_t mutex;
#endif
};

void engine_open(struct engineconnection *ec, const struct uciengine *ue);

int engine_close(struct engineconnection *ec);

timepoint_t engine_readyok(struct engineconnection *ec);

int engine_isready(struct engineconnection *ec);

int engine_hasbestmove(struct engineconnection *ec);

int engine_awaiting(struct engineconnection *ec);

void engine_reset(struct engineconnection *ec);

void engine_seterror(struct engineconnection *ec, int err);

int engine_error(struct engineconnection *ec);

#endif
