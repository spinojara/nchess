#include "timepoint.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
		tc->total = d * TPPERSEC;
		return tc;
	case '+':
		next = 1;
		tc->total = d * TPPERSEC;
		break;
	case ':':
		next = 0;
		/* Only accept integer minutes. */
		for (int i = 0; str[i] != ':'; i++)
			if (str[i] < '0' || str[i] > '9')
				return NULL;
		tc->total = d * 60 * TPPERSEC;
		break;
	default:
		return NULL;
	}

	str = endptr + 1;

	errno = 0;
	d = strtod(str, &endptr);
	if (errno || d < 0)
		return NULL;

	switch (*endptr) {
	case '\0':
		if (next)
			tc->inc = d * TPPERSEC;
		else {
			if (strlen(str) != 2 || d >= 60 || str[0] < '0' || str[0] > '9' || str[1] < '0' || str[1] > '9')
				return NULL;
			tc->total += d * TPPERSEC;
		}
		return tc;
	case '+':
		if (next)
			return NULL;
		else {
			if (strchr(str, '+') - str != 2 || d >= 60 || str[0] < '0' || str[0] > '9' || str[1] < '0' || str[1] > '9')
				return NULL;
			tc->total += d * TPPERSEC;
		}
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
		tc->inc = d * TPPERSEC;
		return tc;
	default:
		return NULL;
	}
}

char *timepoint_str(char *str, int n, timepoint_t t) {
	if (t < 0)
		t = 0;
	timepoint_t minutes = t / (TPPERSEC * 60);
	timepoint_t seconds = (t - TPPERSEC * 60 * minutes) / TPPERSEC;
	timepoint_t milliseconds = (t - TPPERSEC * (seconds + 60 * minutes)) / (100 * TPPERMS);

	if (minutes)
		snprintf(str, n, "%lld:%02lld", minutes, seconds);
	else if (seconds < 10)
		snprintf(str, n, "%lld.%lld", seconds, milliseconds);
	else
		snprintf(str, n, "%lld", seconds);

	return str;
}
