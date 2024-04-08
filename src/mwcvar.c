/*
 * MWC K&R C version of error message functions.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "asm.h"

/*
 * Show line numbers if no listing.
 */
static void
showLine()
{
	if (!((NULL == inpc) || lswitchX))
		fprintf(errdev, "%d: %s: ", inpc->lineNo, inpc->name);
}

/*
 * Print error msg and die.
 */
/*VARARGS1*/
void fatal(const char *fmt, ...) {
	va_list ap;
	int save = errno;

	if (Qswitch)
		exit(1);

	showLine();
	va_start(ap, fmt);
	vfprintf(errdev, fmt, ap);
	fputs("\n", errdev);

	if (0 != (errno = save))
		perror("errno reports");

	exit(1);
}

/*
 * Print error msg to listing.
 */
/*VARARGS1*/
void yyerror(const char *fmt, ...) {
	va_list ap;
	if (2 != pass)
		return;
	errCt++;
	sTitle();

	if (Qswitch)
		return;

	showLine();
	va_start(ap, fmt);
	vfprintf(errdev, fmt, ap);
	fputs("\n", errdev);
}

/*
 * Print warning msg to listing.
 */
/*VARARGS1*/
void yywarn(const char *fmt, ...)
{
	va_list ap;
	if ((2 != pass) || wswitch || Qswitch)
		return;
	sTitle();

	showLine();
	fputs("Warning ", errdev);
	va_start(ap, fmt);
	vfprintf(errdev, fmt, ap);
	fputs("\n", errdev);
}
