#include "intsize.h"

/* fake version of ldiv() */
typedef struct ldiv_t { i32_t quot, rem; } ldiv_t;
static ldiv_t ldiv(i32_t numer, i32_t denom)
{
	ldiv_t l;
	l.quot = numer / denom;
	l.rem = numer % denom;
	return l;
}

/*-----------------------------------------------------------------*/
/* Efficient and Portable Combined Random Number Generators */
/* Pierre L'Ecuyer */
/* CACM V. 31 N. 6 pp. 742-749, 774, June 1988 */

/* This random number generator has a period of 2e18. */
/* It can be computed with 2 divides and 4 multiplies */
/* where divides return 16 bit quotient and remainder */
/* of a 32 bit number divided by a 16 bit number, and */
/* mulitplies return a 32 bit product of two 16 bit */
/* factors.  The ANSI portable implementation is less than */
/* optimal, since it must be implemented using i32_t arithmetic, */
/* but at least the algorithm can be specified. */

static i32_t s1 = 1;
static i32_t s2 = 1;

#if 0  /* Unused. */
void srandl(seed1, seed2) u32_t seed1, seed2;
{
	s1 = seed1 % (i32_t)2147483562L + 1;
	s2 = seed2 % (i32_t)2147483398L + 1;
}
#endif

/* uniform distribution in [1 .. 2147483562] */
i32_t randl()
{
	ldiv_t l;
	register i32_t t;

	l = ldiv(s1, (i32_t)53668L);
	if ((s1 = (i32_t)40014L * l.rem - 12211 * l.quot) < 0)
		s1 += (i32_t)2147483563L;
	l = ldiv(s2, (i32_t)52774L);
	if ((s2 = (i32_t)40692L * l.rem - 3791 * l.quot) < 0)
		s2 += (i32_t)2147483399L;
	if ((t = s1 - s2) < 1)
		t += (i32_t)2147483562L;
	return t-1;
}
