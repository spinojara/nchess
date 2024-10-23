#include "timepoint.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

struct timecontrol *timecontrol_string(struct timecontrol *tc, const char *str) {
	tc->total = 0;
	tc->inc = 0;
	tc->moves = 0;
	tc->infinite = !strcmp(str, "inf");
	if (tc->infinite)
		return tc;

	char *endptr;

	if (strchr(str, '/')) {
		errno = 0;
		long long d = strtoll(str, &endptr, 10);
		if (errno || d <= 0 || *endptr != '/')
			return NULL;
		tc->moves = d;
		str = endptr + 1;
	}

	double d;

	errno = 0;
	d = strtod(str, &endptr);
	if (errno || d <= 0)
		return NULL;

	int next = 0;

	switch (*endptr) {
	case '\0':
		tc->total = d * 1000;
		return tc;
	case '+':
		next = 1;
		tc->total = d * 1000;
		break;
	case ':':
		next = 0;
		tc->total = d * 60 * 1000;
		break;
	default:
		return NULL;
	}

	str = endptr + 1;

	errno = 0;
	d = strtod(str, &endptr);
	if (errno || d <= 0)
		return NULL;

	switch (*endptr) {
	case '\0':
		if (next)
			tc->inc = d * 1000;
		else
			tc->total += d * 1000;
		return tc;
	case '+':
		if (next)
			return NULL;
		else
			tc->total += d * 1000;
		break;
	default:
		return NULL;
	}

	str = endptr + 1;

	errno = 0;
	d = strtod(str, &endptr);
	if (errno || d <= 0)
		return NULL;
	
	switch (*endptr) {
	case '\0':
		tc->inc = d * 1000;
		return tc;
	default:
		return NULL;
	}
}
