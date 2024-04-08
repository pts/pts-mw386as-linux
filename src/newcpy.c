/*
 * Create a new copy of a string.
 */

#include <string.h>

char *alloc(unsigned n);

char *
newcpy(s)
register char *s;
{
    if (NULL == s)
    	return (NULL);
    return (strcpy(alloc(strlen(s) + 1), s));
}
