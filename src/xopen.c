/*
 * Open a file or die in the attempt.
 */
#include <stdio.h>

void fatal(const char *fmt, ...);

FILE *xopen(const char *fn, const char *acs)
{
	FILE *tmp;

	if (NULL == (tmp = fopen(fn, acs)))
		fatal("Cannot fopen(%s, %s)", fn, acs);
	return (tmp);
}
