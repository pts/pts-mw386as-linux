/*
 * 80386 assembler common functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asm.h"
#include "intsize.h"

/*
 * Open a file or die in the attempt.
 */
FILE *
xopen(fn, acs)
const char *fn, *acs;
{
	FILE *tmp;

	if (NULL == (tmp = fopen(fn, acs)))
		fatal("Cannot fopen(%s, %s)", fn, acs); /**/
	return (tmp);
}

/*
 * Make a new copy of a string into tmp space.
 * in temp space on a struct.
 */
char *
gcpy(id, disp)
register char *id;
register unsigned disp;
{
	register char *tmp;

	tmp = galloc(strlen(id) + 1 + disp);
	strcpy(tmp + disp, id);
	return(tmp);
}

/*
 * Make a new copy of a string.
 * in temp space on a struct.
 */

char *
scpy(id, disp)
register const char *id;
register unsigned disp;
{
	register char *tmp;

	tmp = alloc(strlen(id) + 1 + disp);
	strcpy(tmp + disp, id);
	return(tmp);
}

/*
 * Trim surrounding spaces and tabs.
 */
char *
trim(s)
char *s;
{
	register char *p, c;

	for(p = strchr(s, '\0'); --p > s; )
		if((' ' == (c = *p)) || ('\t' == c))
			*p = '\0';
		else
			break;

	for(p = s; (' ' == (c = *p)) || ('\t' == c); p++)
		;
	return(p);
}

/*
 * Count structs with a next or prev pointer  as the first item.
 */
short
countList(p)
const parm *p;
{
	register short n;

	for(n = 0; NULL != p; p = p->next)
		n++;

	return(n);
}

/*
 * Find a parm by name. Return it or NULL.
 */
char *
lookList(t)
const char *t;
{
	short n;
	register parm *p;

	for((p = trueMac->names), (n = 0); NULL != p; p = p->next) {
		if(!strcmp(t, p->str))
			return(parmFind(n, trueMac->parms));
		n++;
	}
	if(!strcmp(".macno", t)) {
		static char buf[2 + sizeof(int) * 3];

		sprintf(buf, "%d", trueMac->expno);
		return(buf);
	}
	return(NULL);
}

/*
 * Find parm n.
 */
char *
parmFind(short n, register parm *p)
{
	for(; NULL != p; p = p->next)
		if(!n--)
			return(p->str);
	return("");
}

/*
 * Do shift operation.
 */
void
doShift(short n)
{
	register parm *p, **pp;
	short sav = n;

	if(NULL == trueMac)
		yyerror(".shift not in macro");
		/* \fB.shift\fR shifts macro parameters.
		 * It has no meaning outside a macro. */
	for(pp = &(trueMac->parms); NULL != (p = *pp); pp = &(p->next)) {
		if(--n == 0) {
			*pp = p->next;
			free((char *)p);
			return;
		}
	}
	yyerror("Parm %d not found", sav);
	/* An attempt to \fB.shift\fR too far has been made. */
}

/*
 * push a new logic level.
 */
void
newLevel(int type)
{
	macctl *mac;

	mac = new(macctl);
	mac->type = type;
	if(NULL != trueMac)
		mac->expno = trueMac->expno;
	mac->prev = logic;
	logic  = mac;
}

/*
 * Pop a logic level.
 */
void
freeLevel(void)
{
	macctl *tmp;

	logic = (tmp = logic)->prev;
	free((char *)tmp);
}

/*
 * Open a file
 */
void
fileOpen(fn)
const char *fn;
{
	inpctl *ip;

	/*
	 * NIGEL: Hack this up so that after the string is copied into the
	 * temp space we try and strip quote-marks. This job made a little
	 * harder because Charles was a perverse son-of-a-bitch and used
	 * flex-arrays.
	 */

	ip = (inpctl *)scpy(fn, offset(inpctl, name));

	if (* ip->name == '"') {
		int		len = strlen (ip->name);

		if (ip->name [len - 1] == '"') {
			strncpy (ip->name, fn + 1, len - 2);
			ip->name [len - 2] = 0;
		}
	}

	ip->fp = xopen (ip->name, "r");
	ip->prev = inpc;
	inpc = ip;
}

/*
 * Check for no truncation to short.
 */
int ck16(n)
register i32_t n;
{
	register short i;

	/* PMC: LINT: OK. Intentional Loss of accuracy. */
	i = n;
	return(i != n);
}

/*
 * Build a label.
 */
void
buildlab(label)
const parm *label;
{
	if(NULL != label)
		symLookUp(label->str, S_LOCAL, dot.loc, dot.sg);
}

/*
 * Label ignored message.
 */
void
labelIgnored(label)
const parm *label;
{
	if (NULL != label)
		yyerror("Label ignored");
		/* This statement cannot take a label. */
}

/*
 * Table error detected.
 */
void kindErr(unsigned short kind)
{
	fatal("Table error kind %x detected", kind); /* TECH */
}

/*
 * Find a name on a char ** and return index.
 */
int nameList(n, nl)
register char *n, **nl;
{
	register int i;

	for(i = 0; (NULL != *nl) && strcmp(n, *nl); i++, nl++)
		;
	return(i);
}
