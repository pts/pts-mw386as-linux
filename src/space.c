/*
 * Space managment for 386 assembler.
 *
 * Normally anything galloc()ed for a line will be free()d
 * at the end of line. This can be avoided by umark()
 */
#include <stdio.h>
#include <asm.h>

static char **mArray;		/* all items galloc()ed for this line */
static unsigned top = 0;	/* highest item galloc()ed this line */
static unsigned max = 64;	/* count of slots in mArray, may grow */

#define QTOP 10			/* count of preallocated blocks */
typedef union quick quick;
union quick {	/* for allocating quickly but wastefully */
	expr	e;
	data	d;
	sym	s;
};

static quick *qb;		/* pre allocated quick blocks */
static unsigned qtop = 0;	/* highest used quick block */

/*
 * Allocate a block of space.
 * Leave if there is no space left
 * at all.
 */
char *
alloc(n)
unsigned n;
{
	register char *p;
	extern char *calloc();

	if ((p = (char *)calloc(n, 1)) == NULL)
		fatal("Out of space");
		/* A call to \fBmalloc()\fR failed.
		 * The typical large consumers of RAM are macros
		 * and \fB.defines\fR; symbols consume less.
		 * Can you break your assembly into
		 * smaller pieces?
		 * Could you be in some sort of
		 * endless recursion or loop? */
	return (p);
}

/*
 * Init storage functions.
 */
void
initStor()
{
	char *j;
	extern char *malloc();

#if 0
	/* This speeds things up on many systems by minamizing
	 * traffic with the operating system for space.
	 */
	if(NULL != (j = malloc(16000)))
		free(j);
#endif
	mArray = (char **)alloc(max * sizeof(*mArray));
	qb = (quick *)alloc(QTOP * sizeof(*qb));
}

/*
 * Expand an area.
 */
expand(area, current, by, size)
char **area;
unsigned *current, by, size;
{
	extern char *realloc();

	if (NULL == (*area = realloc(*area, size * (*current += by))))
		fatal("Out of space");	/* NODOC */
}

/*
 * Get space and keep track of it on mArray.
 */
char *
galloc(size)
unsigned size;
{
	register char *p;

	if(top == max)
		expand(&mArray, &max, 10, sizeof(*mArray));

	return (mArray[top++] = alloc(size));
}

/*
 * Forget an item on mArray.
 */
void
umark(p)
char *p;
{
	register char **t;

	for (t = mArray + top; t != mArray;) {
		if(p == *--t) {
			*t = NULL;
			return;
		}
	}
	fatal("Logic error in umark"); /* TECH */
}

/*
 * free all items on mArray.
 */
void
freel()
{
	register char **t;

	for(t = mArray + top; t != mArray; )
		if(NULL != *--t)
			free(*t);
	qtop = top = 0;
}

/*
 * unmark all the parms.
 */
void
umList(p)
register parm *p;
{
	for(; NULL != p; p = p->next)
		umark((char *)p);
}

/*
 * free list of items connected by next pointers.
 */
void
freeList(p)
register parm *p;
{
	register parm *t;

	while(NULL != p) {
		p = (t = p)->next;
		free((char *)t);
	}
}

/*
 * Get an expr block.
 * There can only be more than 4 on an error to be spotted later.
 * This is done because the profiler spotted a large amount of time
 * malloc()ing expr blocks.
 */
expr *
xalloc()
{
	register expr *tmp;

	if(qtop == QTOP)
		return(gnew(expr));
	tmp = &(qb[qtop++].e);
	return((expr *)clear(tmp));
}

/*
 * Make a temporary copy of a sym.
 * Rarely done more than twice a line.
 */
sym *
copySym(s)
sym *s;
{
	register sym *n;

	if(qtop == QTOP)
		n = gnew(sym);
	else
		n = &(qb[qtop++].s);
	*n = *s;
	return(n);
}

/*
 * Get space for data item.
 */
data *
gitem(type)
{
	register data *d;

	if(qtop == QTOP)
		d = gnew(data);
	else
		d = &(qb[qtop++].d);
	d->next = NULL;
	d->type = type;
	d->count = 1;
	return(d);
}
