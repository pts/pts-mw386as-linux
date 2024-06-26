/*
 * libc/gen/ieee_d.c
 */

#include <string.h>

/*
 * ieee_d(double *idp, double *ddp)
 * Convert double from DECVAX format to IEEE format.
 * ddp points to a DECVAX format double to convert.
 * idp points to a destination for the converted IEEE value;
 * idp may be identical to ddp for in-place conversion.
 * The DECVAX significand is truncated, not rounded.
 * Always returns 0, because the conversion always succeeds.
 */

int
ieee_d(idp, ddp) double *idp, *ddp;
{
	unsigned char a[8];
	register unsigned char *cp;
	register int exp;

	/* Extract biased exponent. */
	cp = (unsigned char *)ddp;
	exp = ((cp[7] & 0x7F) << 1) | ((cp[6] & 0x80) >> 7);

	if (exp == 0) {
		memset((char *)idp, 0, 8);	/* set idp to 0.0 */
		return 0;
	}

	/* Adjust exponent bias and repack. */
	exp += 1023 - 129;
	a[7] = (cp[7] & 0x80) | (exp >> 4);
	a[6] = ((exp & 0x0F) << 4)  | ((cp[6] & 0x78) >> 3);
	a[5] = ((cp[6] & 0x7) << 5) | ((cp[5] & 0x7F) >> 3);
	a[4] = ((cp[5] & 0x7) << 5) | ((cp[4] & 0x7F) >> 3);
	a[3] = ((cp[4] & 0x7) << 5) | ((cp[3] & 0x7F) >> 3);
	a[2] = ((cp[3] & 0x7) << 5) | ((cp[2] & 0x7F) >> 3);
	a[1] = ((cp[2] & 0x7) << 5) | ((cp[1] & 0x7F) >> 3);
	a[0] = ((cp[1] & 0x7) << 5) | ((cp[0] & 0x7F) >> 3);

	/* Done, copy to destination. */
	memcpy((char *)idp, a, 8);
	return 0;
}

/* end of libc/gen/ieee_d.c */
