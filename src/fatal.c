/*
 * Put message and die.
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void fatal(const char *fmt, ...)
{
	va_list ap;
	fputs("\nfatal: ", stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	exit(1);
}
