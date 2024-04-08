/*
 * Get space or die.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void fatal(const char *fmt, ...);

char *
alloc(n)
unsigned n;
{
	char	*tmp;

	if (NULL == (tmp = malloc(n)))
		fatal("Out of space");
	return(memset(tmp, '\0', n));
}
