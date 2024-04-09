/*
 * Initialize the assembler
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "asm.h"

void yyparse(void);

#if !defined(MSDOS) && !defined(_WIN32)
#define SLASH '/'
#else
#define SLASH '\\'  /* TODO(pts): Smarter, accept both / and \ on Windows. */
#endif

#if 0
	/* This is used only to get the Syntax error message into the docs. */

	yyerror("Syntax error");
	/* The syntax of this statement makes no sense to the parser.
	 * This can be a variety of problems. */
#endif

static char swChars[] = "VwQXaxfpbgnlo:?E:D:"; /* list for getargs */
static int savArgc;
static char **savArgv;
static char dodefsw;	/* -D or -E used */

/*
 * Do -E and -D definitions. 
 * -Dx=y is the same as x .define y
 * -Ea=5 is the same as a .equ 5
 * This is called every pass.
 */
void
dodefs()
{
	register char *p, c;

	if (!dodefsw)	/* no -D or -E to process */
		return;

	optix = 1;	/* reset getargs */
	while ((char)EOF != (c = getargs(savArgc, savArgv, swChars))) {
		switch(c) {
		case 'D':
		case 'E':
			if (NULL == (p = strchr(as_optarg, '=')) || p == as_optarg)
				fatal("Invalid option on -%c switch", c);
				/* The syntax of -D and -E switches is
				 * -Dname=string
				 * -Ename=number */
			*p++ = '\0';
			if ('D' == c) {
				defCt++;
				defMac(gcpy(as_optarg, 0),
				       gcpy(p, offset(parm, str)),
				       MACSTR);
			}
			else
				symLookUp(gcpy(as_optarg, 0), S_XSYM, strtol(p, NULL, 10), 0);
			*--p = '=';
			break;
		default: break;
		}
	}
}

/*
 * Invalid usage message.
 */
void
usage()
{
	fprintf(stderr, "usage: as [-wXxfpbgnlQ] [-o output] filename\n");
	exit(1);
}

/*
 * process arguments and call yyparse.
 */
int main(int argc, char **argv)
{
	register char *p, c;
	char *fileName;

	errdev = stderr;	/* errors normally go to stderr */

	initStor();		/* init storage control */
	segInit();		/* init segment data */
	indPass();		/* init indefinite branch logic */
	newLevel(NORMAL);	/* set base level for if else etc */

	_addargs("AS", &argc, &argv);
	savArgv = argv;
	savArgc = argc;

	fileName = NULL;
	while ((char)EOF != (c = getargs(argc, argv, swChars))) {
		switch(c) {
		case 0:
			if (NULL != fileName)	/* one time only */
				fatal("more than one file to process");
				/* The assembler will only process one file
				 * at a time. */
			fileName = as_optarg;
			break;

		case 'a':	/* align data objects */
			alignon = alignonX ^= 1;
			break;

		case 'n':	/* No fixes for chip errata */
			nswitch = nswitchX ^= 1; 
			break;

		case 'w':	/* No warning messages */
			wswitchX ^= 1;	
			break;

		case 'X':	/* don't output .L local symbols */
			Xswitch ^= 1;	
			break;

		case 'x':	/* no local symbols in object file */
			xswitch ^= 1;	
			break;

		case 'f':	/* reverse order of operands */
			fswitch = fswitchX ^= 1; 
			break;

		case 'p':	/* don't use % on register names */
			rswitch ^= 1;	
			break;

		case 'b':	/* reverse bracket sense */
			bswitch = bswitchX ^= 1; 
			break;

		case 'Q':	/* No messages at all */
			Qswitch ^= 1;
			break;

		case 'g':	/* treat unrefrenced symbols as globl */
			gswitch = 1;	
			break;

		case 'l':	/* print a listing */
			errdev = (lswitchX ^= 1) ? stdout : stderr;
			break;

		case 'o':
			outName = as_optarg;
			if ((NULL == (p = strrchr(outName, '.'))) ||
			    strcmp(p, ".o"))
				fatal("Unlikely output file '%s'", outName);
			/* Output file-names should have \fB.o\fR suffixes.
			 * Because this is generally a typographical error,
			 * \fBas\fR aborts to
			 * avoid overwriting an important file. */
			break;

		case 'D':	/* process in dodefs */
		case 'E':
			dodefsw = 1;
			break;
		case 'V':
			fprintf(stderr, "Mark Williams 80386 assembler\n");
			break;

		case '?':
		default:
			usage();
		}
	}

	if (fileName == NULL)
		fatal("no work");
		/* There were no files listed on the command line. */

	symInit();		/* init symbol table */

	fileOpen(fileName);
	title = scpy(fileName, 0);

	if(NULL == outName) {	/* develop output filename */
		char *q;
		int len;

		/* point p at input file name */
		if (NULL != (q = strrchr(p = fileName, SLASH)))
			p = q + 1;

		outName = alloc((unsigned)(3 + (len = strlen(p))));
		strcpy(outName, p);

		p = outName + len - 2;	/* right spot for .  */
		if ('.' != *p)		/* if no . append .o */
			p += 2;

		strcpy(p, ".o");
	}

	/* get ctime without \n */
	{
		union { time_t t; i32_t i[2]; } tt;

		tt.i[1] = 0;  /* Defensive programming in case libc .h is 32 bits and libc code is 64 bits. */
		time(&tt.t);
		dTime = ctime(&tt.t);
		dTime[strlen(dTime) - 1] = '\0';
	}

	dodefs();
	yyparse();
	fatal("Unexpected return from parser"); /* TECH */
	return -1;  /* Not reached. */
}
