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

#define TPPERSEC 1000000000l
#define TPPERMS     1000000l

static inline timepoint_t time_now(void) {
	struct timespec tp;
	clock_gettime(CLOCK, &tp);
	return (timepoint_t)tp.tv_sec * TPPERSEC + (timepoint_t)tp.tv_nsec;
}

static inline timepoint_t time_since(timepoint_t t) {
	return time_now() - t;
}

#endif
