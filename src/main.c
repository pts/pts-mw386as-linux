/*
 * Initialize the assembler
 */
#include <asm.h>

#ifdef COHERENT
#define SLASH '/'
#else
#define SLASH '\\'
#endif

#if 0
	/* This is used only to get the Syntax error message into the docs. */

	yyerror("Syntax error");
	/* The syntax of this statement makes no sense to the parser.
	 * This can be a variety of problems. */
#endif

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
main(argc, argv)
char **argv;
{
	register char *p, c;
	char *fileName;
	extern char *optarg;

	initStor();		/* init storage control */
	segInit();		/* init segment data */
	indPass();		/* init indefinite branch logic */
	newLevel(NORMAL);	/* set base level for if else etc */

	_addargs("AS", &argc, &argv);
	fileName = NULL;
	while (EOF != (c = getargs(argc, argv, "wQXxfpbgnlo:?"))) {
		switch(c) {
		case 0:
			if (NULL != fileName)	/* one time only */
				fatal("more than one file to process");
				/* The assembler will only process one file
				 * at a time. */
			fileName = optarg;
			break;

		case 'n':	/* No fixes for chip errata */
			nswitch = nswitchX ^= 1; 
			break;

		case 'Q':	/* No messages at all */
			Qswitch ^= 1;

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

		case 'g':	/* treat unrefrenced symbols as globl */
			gswitch = 1;	
			break;

		case 'l':	/* print a listing */
			lswitchX ^= 1;	
			break;

		case 'o':
			outName = optarg;
			if ((NULL == (p = strrchr(outName, '.'))) ||
			    strcmp(p, ".o"))
				fatal("Unlikely output file '%s'", outName);
			/* Output file-names should have \fB.o\fR suffixes.
			 * Because this is generally a typographical error,
			 * \fBas\fR aborts to
			 * avoid overwriting an important file. */
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

		if (NULL != (q = strrchr(p = fileName, SLASH)))
			p = q + 1;

		outName = alloc((unsigned)(strlen(p) + 3));
		strcpy(outName, p);
		if(NULL == (p = strrchr(outName, '.'))) {
			p = strrchr(outName, 0);
			*p++ = '.';
			*p++ = 'o';
		}
		else {
			*++p = 'o';
			*++p = '\0';
		}
	}

	/* get ctime without \n */
	{
		long t;

		time(&t);
		c = strlen(dTime = ctime(&t)) - 1;
		dTime[c] = '\0';
	}

	yyparse();
	fatal("Unexpected return from parser"); /* TECH */
}
