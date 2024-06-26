/*
 * getargs - get option letters and arguments from argv
 *
 * This is an improved form of getopt. If an option is followed
 * by an ! it has an optional argument.
 * 	while(EOF != (c = getargs(argc, argv, "xyf:g!")))
 * Is a call of getargs from its test section. The x and y
 * options take no arguments. The f option takes a mandidory
 * argument, -f arg, and -farg, are both legal forms. The g option
 * takes an optional argument which if present must be connected
 * -garg. Additional arguments are returned as if they were preceeded
 * by an option of '\0'. This allows programs such as ld and as to
 * process mixed options and file names.
 */
#include <stdio.h>
#include <string.h>

void fatal(const char *fmt, ...);

char	*as_optarg;	/* Global argument pointer. */
int	optix = 1;	/* Global argv index. Reset to 1 to rescan. */

static char	*scan = NULL;	/* Private scan pointer. */

int getargs(int argc, char * const argv[], const char *optstring)
{
	register char c, a;
	register const char *place;

	for (as_optarg = NULL; scan == NULL || !*scan; scan++, optix++) {
		if (optix >= argc) {
			scan = NULL;
			return(EOF);
		}
		if (*(scan = argv[optix]) != '-') {
			as_optarg = scan;
			scan = NULL;
			optix++;
			return (0);
		}
	}

	if ((place = strchr(optstring, c = *scan++)) == NULL ||
	     c == ':' || c == '!')
		fatal("Unknown command option %c", c);
		/**/
	if (((a = place[1]) == ':') || (a == '!')) {
		if (*scan || (a == '!')) {
			as_optarg = scan;
			scan = NULL;
		} else if (optix < argc)
			as_optarg = argv[optix++];
		else
			fatal("Command option '%c' missing its argument", c);
			/**/
	}

	return(c);
}

#ifdef TEST
/*
 * This test example shows how to use getargs in a program.
 * Typical test lines are
 * getargs -fxxx -f yyy a b c -x -gabc -g
 * getargs -xj	# invalid stuff
 */
main(argc, argv)
char *argv[];
{
	char c;

	while(EOF != (c = getargs(argc, argv, "xyf:g!"))) {
		switch(c) {
		case 'x':
		case 'y':
			printf("option %c\n", c);
			break;
		case 'g':
			if (*as_optarg)
				printf("option g with %s\n", as_optarg);
			else
				printf("option g with no argument\n");
			break;
		case 'f':
			printf("option f with %s\n", as_optarg);
			break;
		case 0:
			printf("argument '%s'\n", as_optarg);
			break;
		default:
			printf(
		"usage: getargs [-xy] [-f filen] [-g[option]] name ...\n");
			exit(1);
		}
	}
	exit(0);
}
#endif
