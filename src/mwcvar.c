/*
 * MWC K&R C version of error message functions.
 */
#include <asm.h>
#include <errno.h>

/*
 * Print error msg and die.
 */
/*VARARGS1*/
void
fatal(s)
char *s;
{
	int save = errno;

	if (Qswitch)
		exit(1);

	if ((NULL == inpc) || lswitchX)
		printf("%r\n", &s);
	else
		printf("%d: %s: %r\n", inpc->lineNo, inpc->name, &s);

	if (0 != (errno = save))
		perror("errno reports");

	exit(1);
}

/*
 * Print error msg to listing.
 */
/*VARARGS1*/
void
yyerror(s)
char *s;
{
	if (2 != pass)
		return;
	errCt++;
	sTitle();

	if (Qswitch)
		return;

	if ((NULL == inpc) || lswitchX)
		printf("%r\n", &s);
	else
		printf("%d: %s: %r\n", inpc->lineNo, inpc->name, &s);
}

/*
 * Print warning msg to listing.
 */
/*VARARGS1*/
void
yywarn(s)
char *s;
{
	if ((2 != pass) || wswitch)
		return;
	sTitle();

	if ((NULL == inpc) || lswitchX)
		printf("Warning %r\n", &s);
	else
		printf("%d: %s: Warning %r\n", inpc->lineNo, inpc->name, &s);
}
