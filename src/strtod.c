/*
 * Transform a number from ascii to double precision.
 * Uses no knowledge of floating representation, except for
 * the fact that 2 longs hold more precision than a double.
 * Replace where system provides this.
 */
#include <stdio.h>
#include "utype.h"
#define LONG_MAX 0x7fffffffL
#define SAFELONG ((LONG_MAX - 9) / 5)
extern double _pow10();

#define NEG	1
#define DOT	2
#define NEGEXP	4
#define BIG	8
#define VBIG	16

double
strtod(sp, endp)
char *sp, **endp;
{
	double r;
	unsigned long acc, acc1, w;
	register int flags, c, n, decexp;

	decexp = acc = flags = 0;
	while(isascii(c = *sp++) && isspace(c))
		;
	switch(c) {
	case '-':
		flags |= NEG;
	case '+':
		c = *sp++;
	}
	for(;;c = *sp++) {
		if(isdigit(c)) {
			if(flags & DOT)
				decexp--;
			c -= '0';
			if(flags & BIG)
				if(flags & VBIG)  /* more precision than */
					decexp++; /* a double will hold */
				else {
					n++;
					w = acc << 1;
					if(SAFELONG<(acc = w + ((w << 2) + c)))
						flags |= VBIG;
				}
			else {
				w = acc << 1;
				if(SAFELONG < (acc = w + ((w << 2) + c))) {
					flags |= BIG;
					acc1 = acc;
					acc = n = 0;
				}
			}
		}
		else if(('.' == c) && !(flags & DOT))
			flags |= DOT;
		else
			break;
	}
	r = acc;
	if(flags & BIG)
		r += _pow10(n) * (double)acc1;
	if(flags & NEG)
		r = -r;

	if('e' == c || 'E' == c) {
		switch(c = *sp++) {
		case '-':
			flags |= NEGEXP;
		case '+':
			c = *sp++;
		}
		for(n = 0; isdigit(c); c = *sp++)
			n = (10 * n) + (c - '0');
		if(flags & NEGEXP)
			decexp -= n;
		else
			decexp += n;
	}
	if(NULL != endp)
		*endp = sp - 1;
	if(decexp)
		return(r * _pow10(decexp));
	return(r);
}
