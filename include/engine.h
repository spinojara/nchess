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
	TYPE_STRING,
	TYPE_BUTTON,
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
	char *value;
	int type;
	char *min;
	char *max;
	char *def;
};

struct uciengine {
	char *name;
	char *command;
	char *workingdir;
	struct ucioption *ucioptions;
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
