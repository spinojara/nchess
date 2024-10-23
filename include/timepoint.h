#ifndef TIMEPOINT_H
#define TIMEPOINT_H

#include <time.h>

#ifdef CLOCK_MONOTONIC_RAW
#define CLOCK CLOCK_MONOTONIC_RAW
#else
#define CLOCK CLOCK_MONOTONIC
#endif

/* timepoint_t is given in nanoseconds. */
typedef long long timepoint_t;

#define TPPERSEC 1000000000ll
#define TPPERMS     1000000ll

static inline timepoint_t time_now(void) {
	struct timespec tp;
	clock_gettime(CLOCK, &tp);
	timepoint_t ret = (timepoint_t)tp.tv_sec * TPPERSEC + (timepoint_t)tp.tv_nsec;
	return ret ? ret : 1;
}

static inline timepoint_t time_since(timepoint_t t) {
	return time_now() - t;
}

struct timecontrol {
	timepoint_t total;
	timepoint_t inc;
	long long moves;

	int infinite;
};

struct timecontrol *timecontrol_string(struct timecontrol *timecontrol, const char *str);

#endif
