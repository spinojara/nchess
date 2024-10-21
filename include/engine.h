#ifndef ENGINE_H
#define ENGINE_H

#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>

#include "timepoint.h"

enum {
	TYPE_CHECK,
	TYPE_SPIN,
	TYPE_STRING,
	TYPE_BUTTON,
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
	pthread_t tid;
	FILE *w, *r;

	timepoint_t bestmovetime;
	pthread_mutex_t mutex;
};

void engine_open(struct engineconnection *ec, struct uciengine *ue);

void engine_close(struct engineconnection *ec);

#endif
