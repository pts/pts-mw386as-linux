
#line 1 "asm.y"

#include <asm.h>
/*
 * count macro parms.
 */
static 
parmCt()
{
	if(NULL == trueMac) {
		yyerror(".parmct not in macro");
 		/* \fB.parmct\fR returns the number of
		 * parameters in the current macro. */
		return(0);
	}
	else
		return(countList(trueMac->parms));
}

/*
 * Verify that 2 symbols have the same segment.
 */
static void
ckseg(s1, s2)
sym *s1, *s2;
{
	if(s1->sg != s2->sg)
		yyerror("Arithmetic between addresses on different segments");
		/* You may only add or subtract addresses if they
		 * are in the same segment. */
}

/*
 * Create Immediate exps.
 */
static expr *
setImm(val, symRef)
long val;
sym  *symRef;
{
	register expr *oper;
	register short w;

	oper = xalloc();
	oper->mode = T_IMM;
	if (NULL != (oper->ref = symRef))
		oper->ref = symRef->ref;
	w = oper->exp = val;
	if(w == val)
		oper->size = 2;
	else
		oper->size = 4;
	return(oper);
}

/*
 * Set up for most addressing modes.
 */
static expr *
qbild(val, mode, r1, r2, scale, symRef)
long val;
int mode;
psym *r1, *r2;
long scale;
sym *symRef;
{
	register expr *oper;
	char i;

	if (NULL != r1 && T_R != mode) {
		switch((int)r1->size) {
		case 2:
			lflags |= A_SHORT;
			if (!scale)
				break;
		case 1:
			regerror(r1);
			break;
		case 4:
			lflags |= A_LONG;
			break;
		}

		if (ORD_REG != r1->flag)	/* cant ind via ctl regs */
			regerror(r1);

		if (NULL != r2) {
			if (ORD_REG != r2->flag)
				regerror(r2);
			if (r1->size != r2->size)
				yyerror("Mixed length addressing registers");
		/* Addressing registers must both be the same length. */
		}
	}
	else if ((T_R == mode) && (r1->flag == ORD_REG)) {
		switch ((int)r1->size) {
		case 4:
			lflags |= O_LONG;
			break;
		case 2:
			lflags |= O_SHORT;
		}
	}

	oper = xalloc();
	oper->exp  = val;
	oper->r1 = r1;
	oper->r2 = r2;
	oper->mode = mode;

	i = scale;
	if (i != scale)
		i = 3;	/* set a bad scale */

	switch (i) {
	case 0:
	case 1:	/* for generated code */
		oper->scale = 0; break;
	case 2:
		oper->scale = 1; break;
	case 4:
		oper->scale = 2; break;
	case 8:
		oper->scale = 3; break;
	default:
		yyerror("Bad scale");
		/* Address scale must be 0, 1, 2, 4, or 8. */
	}

	if (NULL != (oper->ref = symRef))
		oper->ref = symRef->ref;
	return(oper);
}

/*
 * Floating point register expr.
 */
static expr *
fbild(regno)
long regno;
{
	register expr *oper;

	if (regno < 0 || regno > 7) {
		regno &= 7;
		yyerror("Invalid floating point register number"); /**/
	}

	oper = xalloc();
	oper->exp = regno;
	oper->mode = T_FP;
	return (oper);
}
	
/*
 * Report register error.
 */
regerror(rg)
psym *rg;
{
	yyerror("%s is an improper register in this context", rg->name); /**/
}

/*
 * Concatinate strings.
 */
static char *
concat(s1, s2)
char *s1, *s2;
{
	long l;
	unsigned short u;
	char *res;

	u = l = (long)strlen(s1) + (long)strlen(s2) + 1;
	if(u != l) {
		yyerror("Length %ld string range exceeded", l);
		/* Strings may not exceed 32 kilobytes. */
		return(galloc(1));
	}
	res = galloc(u);
	sprintf(res, "%s%s", s1, s2);
	return(res);
}

/*
 * Substring.
 * Assumes that all strings are created from unsigned lengths.
 */
static char *
substr(s, from, len)
char *s;
long from, len;
{
	register char *p, *res;
	unsigned short l;

	l = strlen(s);
	s += from;
	from = l - from;	/* from now len to end */

	if(len > from)
		len = from;	/* since strlen fit unsigned len must */
	if(len < 0)
		len = 0;

	for(p = res = galloc((unsigned)(len + 1)); len--; )
		*p++ = *s++;
	return(res);
}

/*
 * String search.
 */
static
stringSearch(s1, s2)
char *s1, *s2;
{
	char *p;

	if(NULL == (p = strstr(s1, s2)))
		return(-1);
	return(p - s1);
}

/*
 * Do long comparisons.
 * < > <= >= != ==  compare operator
 * 1 2  5  6  3  4  t
 */
static
compare(t, v)
int t;
long v;
{
	return(((v < 0) ? t : (v > 0) ? (t >> 1) : (t >> 2)) & 1);
}

/*
 * Do double comparisons.
 * < > <= >= != ==  compare operator
 * 1 2  5  6  3  4  t
 */
static
fcompare(t, v)
int t;
double v;
{
	return(((v < 0.0) ? t : (v > 0.0) ? (t >> 1) : (t >> 2)) & 1);
}

static void
unmatched(c)
{
	yyerror("Unmatched '%c'", c);
	/* A delimeter, [, (, ), or ] is unmatched in this command. */
}


#include "y_tab.h"
#define YYCLEARIN yychar = -1000
#define YYERROK yyerrflag = 0
extern int yychar;
extern short yyerrflag;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
YYSTYPE yyval, yylval;
#ifdef YYTNAMES
readonly struct yytname yytnames[49] =
{
	"$end", -1, 
	"error", -2, 
	"PC", 256, 
	"NUMBER", 257, 
	"FNUM", 258, 
	"COMPARISON", 311, 
	"OP", 260, 
	"DATA", 261, 
	"CMD", 262, 
	"DCMD", 263, 
	"ICMD", 264, 
	"NCMD", 265, 
	"ECMD", 266, 
	"IDENTIFIER", 267, 
	"REG", 268, 
	"DEFINED", 319, 
	"SIZEOF", 320, 
	"SEGMENT", 321, 
	"LOCATION", 322, 
	"PLUS", 314, 
	"MINUS", 315, 
	"TIMES", 316, 
	"DIVIDE", 317, 
	"REM", 318, 
	"LSHIFT", 312, 
	"RSHIFT", 313, 
	"AND", 310, 
	"OR", 308, 
	"XOR", 309, 
	"COMMA", 304, 
	"LPAREN", 327, 
	"RPAREN", 285, 
	"LBRACK", 328, 
	"RBRACK", 287, 
	"AT", 306, 
	"D_SIGN", 289, 
	"NOT", 323, 
	"BANG", 325, 
	"COLON", 305, 
	"PARMCT", 293, 
	"TOSTRING", 329, 
	"TONUMBER", 330, 
	"TOFLOAT", 331, 
	"FSTACK", 297, 
	"NL", 332, 
	"TOKEN", 326, 
	"P_SIGN", 307, 
	"LEN", 324, 
	NULL
} ;
#endif
#include <action.h>
readonly unsigned char yypdnt[106] = {
0, 1, 1, 2, 2, 2, 2, 2,
2, 2, 2, 2, 2, 7, 7, 7,
6, 6, 6, 6, 6, 4, 4, 4,
5, 3, 3, 8, 8, 8, 8, 13,
13, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 14,
14, 14, 14, 14, 14, 14, 14, 9,
9, 9, 9, 9, 11, 11, 11, 11,
11, 11, 11, 11, 11, 11, 11, 11,
11, 11, 11, 11, 11, 11, 11, 11,
11, 11, 11, 11, 11, 11, 11, 11,
11, 11, 12, 12, 12, 12, 12, 12,
12, 10, 10, 10, 10, 10, 10, 10,
10, 10 
};
readonly unsigned char yypn[106] = {
2, 1, 2, 4, 4, 6, 3, 3,
4, 4, 4, 2, 2, 3, 1, 0,
1, 1, 1, 1, 3, 1, 3, 0,
1, 0, 1, 3, 1, 2, 0, 1,
3, 1, 1, 4, 3, 5, 6, 5,
7, 4, 6, 7, 6, 8, 4, 6,
7, 6, 8, 1, 1, 2, 2, 1,
3, 3, 3, 3, 1, 3, 3, 3,
3, 3, 3, 2, 2, 2, 2, 2,
1, 3, 3, 3, 3, 3, 3, 3,
3, 3, 3, 2, 2, 2, 4, 3,
2, 2, 1, 3, 3, 6, 4, 2,
2, 1, 3, 3, 3, 3, 3, 2,
2, 2 
};
readonly unsigned char yypgo[15] = {
0, 0, 2, 6, 8, 14, 18, 26,
30, 34, 70, 106, 190, 218, 222 
};
readonly unsigned int yygo[226] = {
YYOTHERS, 0x3, 0x3, 0x8, YYOTHERS, 0x4, YYOTHERS, 0x5,
0xC, 0x36, 0x79, 0xAD, YYOTHERS, 0x34, 0xF, 0x3A,
YYOTHERS, 0x35, 0xD, 0x37, 0x77, 0xAC, 0x7C, 0xAE,
YYOTHERS, 0x2D, 0x75, 0xAB, YYOTHERS, 0x2E, 0x74, 0xAA,
YYOTHERS, 0x26, 0x9, 0x27, 0xA, 0x2F, 0xD, 0x2F,
0x16, 0x3E, 0x17, 0x40, 0x18, 0x41, 0x1A, 0x27,
0x1B, 0x46, 0x1D, 0x4C, 0x3B, 0x27, 0x5A, 0x8B,
0x5C, 0x8D, 0x64, 0x97, 0x74, 0x27, 0x75, 0x2F,
0x77, 0x2F, 0x7C, 0x2F, YYOTHERS, 0x42, 0xA, 0x30,
0xD, 0x30, 0x19, 0x43, 0x1B, 0x47, 0x21, 0x50,
0x22, 0x54, 0x52, 0x87, 0x53, 0x88, 0x5E, 0x91,
0x5F, 0x92, 0x60, 0x93, 0x61, 0x94, 0x62, 0x95,
0x75, 0x30, 0x77, 0x30, 0x7C, 0x30, 0x86, 0x88,
YYOTHERS, 0x28, 0x9, 0x29, 0x16, 0x3F, 0x17, 0x3F,
0x18, 0x3F, 0x19, 0x44, 0x1A, 0x29, 0x1B, 0x48,
0x1D, 0x4D, 0x1E, 0x4E, 0x1F, 0x4F, 0x21, 0x51,
0x23, 0x56, 0x3B, 0x29, 0x58, 0x8A, 0x5A, 0x3F,
0x5B, 0x8C, 0x5C, 0x8E, 0x63, 0x96, 0x64, 0x98,
0x65, 0x99, 0x66, 0x9A, 0x67, 0x9B, 0x68, 0x9C,
0x69, 0x9D, 0x6A, 0x9E, 0x6B, 0x9F, 0x6C, 0xA0,
0x6D, 0xA1, 0x71, 0xA7, 0x72, 0xA8, 0x74, 0x29,
0x83, 0xB0, 0xB1, 0xC0, 0xB3, 0xC2, 0xB6, 0xC5,
0xB9, 0xC7, 0xBD, 0xC8, 0xC3, 0xCD, 0xC6, 0xD1,
0xCA, 0xD4, 0xCE, 0xD6, YYOTHERS, 0x31, 0xA, 0x32,
0xD, 0x32, 0x1B, 0x49, 0x22, 0x55, 0x23, 0x57,
0x53, 0x89, 0x6F, 0xA5, 0x70, 0xA6, 0x73, 0xA9,
0x75, 0x32, 0x77, 0x32, 0x7C, 0x32, 0xA4, 0x89,
YYOTHERS, 0x2A, 0x1A, 0x45, YYOTHERS, 0x2B, 0x3B, 0x7E,
YYOTHERS, 0x2C 
};
readonly unsigned short yypa[218] = {
0, 20, 24, 26, 48, 50, 68, 70,
74, 76, 120, 154, 154, 158, 192, 196,
202, 204, 206, 208, 210, 214, 158, 158,
158, 158, 220, 158, 262, 158, 158, 158,
268, 158, 270, 158, 284, 288, 290, 294,
304, 316, 342, 354, 358, 360, 364, 368,
376, 388, 414, 426, 428, 432, 436, 440,
444, 446, 448, 452, 494, 496, 498, 500,
524, 526, 528, 536, 538, 540, 542, 552,
566, 592, 606, 612, 616, 624, 648, 650,
652, 654, 656, 666, 680, 682, 684, 686,
158, 688, 158, 158, 158, 690, 656, 656,
656, 656, 656, 158, 158, 158, 158, 158,
158, 158, 158, 158, 158, 158, 696, 702,
702, 158, 158, 710, 76, 120, 718, 158,
720, 154, 722, 724, 158, 726, 728, 730,
732, 734, 736, 738, 774, 776, 656, 536,
780, 792, 800, 826, 832, 840, 842, 850,
856, 860, 870, 876, 882, 884, 886, 902,
904, 912, 920, 922, 924, 926, 938, 950,
968, 990, 1010, 1016, 1020, 1028, 1034, 1038,
1066, 1092, 1098, 1100, 1102, 1104, 1106, 1110,
1116, 158, 1142, 1144, 1180, 1182, 1186, 1222,
1224, 158, 1228, 1230, 1232, 158, 1234, 1236,
1238, 1264, 1270, 158, 1296, 1302, 158, 1328,
1354, 1380, 158, 1382, 1384, 1386, 158, 1412,
1414, 1416, 1442, 1444, 1446, 1472, 1474, 1500,
1502, 1504 
};
0x10, 0x14C, 0x6000, YYOTHERS, 0x200C, YYOTHERS, 0x4000, YYEOFVAL,
0x6000, YYOTHERS, 0x2002, YYOTHERS, 0x11, 0x101, 0x12, 0x102,
0x13, 0x10B, 0x14, 0x10C, 0x15, 0x13F, 0x16, 0x140,
0x17, 0x141, 0x18, 0x142, 0x19, 0x13B, 0x1A, 0x13C,
0x1B, 0x147, 0x1C, 0x148, 0x1D, 0x121, 0x1E, 0x143,
0x1F, 0x145, 0x20, 0x125, 0x21, 0x149, 0x22, 0x14A,
0x23, 0x14B, 0x24, 0x129, 0x25, 0x146, 0x201E, YYOTHERS,
0x11, 0x101, 0x12, 0x102, 0x13, 0x10B, 0x15, 0x13F,
0x16, 0x140, 0x17, 0x141, 0x18, 0x142, 0x19, 0x13B,
0x1B, 0x147, 0x1E, 0x143, 0x1F, 0x145, 0x20, 0x125,
0x21, 0x149, 0x22, 0x14A, 0x23, 0x14B, 0x25, 0x146,
0x200F, YYOTHERS, 0x33, 0x146, 0x2017, YYOTHERS, 0x11, 0x101,
0x12, 0x102, 0x13, 0x10B, 0x15, 0x13F, 0x16, 0x140,
0x17, 0x141, 0x18, 0x142, 0x19, 0x13B, 0x1B, 0x147,
0x1E, 0x143, 0x1F, 0x145, 0x20, 0x125, 0x21, 0x149,
0x22, 0x14A, 0x23, 0x14B, 0x25, 0x146, 0x6000, YYOTHERS,
0x38, 0x14C, 0x6000, YYOTHERS, 0x39, 0x14C, 0x33, 0x146,
0x6000, YYOTHERS, 0x200B, YYOTHERS, 0x203C, YYOTHERS, 0x2061, YYOTHERS,
0x2037, YYOTHERS, 0x3B, 0x131, 0x2021, YYOTHERS, 0x3C, 0x101,
0x3D, 0x10B, 0x6000, YYOTHERS, 0x11, 0x101, 0x12, 0x102,
0x13, 0x10B, 0x14, 0x10C, 0x15, 0x13F, 0x16, 0x140,
0x17, 0x141, 0x18, 0x142, 0x19, 0x13B, 0x1B, 0x147,
0x1C, 0x148, 0x1D, 0x121, 0x1E, 0x143, 0x1F, 0x145,
0x20, 0x125, 0x21, 0x149, 0x22, 0x14A, 0x23, 0x14B,
0x24, 0x129, 0x25, 0x146, 0x6000, YYOTHERS, 0x4A, 0x10C,
0x4B, 0x130, 0x6000, YYOTHERS, 0x2048, YYOTHERS, 0x12, 0x102,
0x52, 0x13B, 0x53, 0x147, 0x21, 0x149, 0x23, 0x14B,
0x25, 0x146, 0x6000, YYOTHERS, 0x58, 0x148, 0x2022, YYOTHERS,
0x205A, YYOTHERS, 0x59, 0x14C, 0x6000, YYOTHERS, 0x5A, 0x137,
0x5B, 0x13A, 0x5C, 0x13B, 0x5D, 0x148, 0x2033, YYOTHERS,
0x5E, 0x137, 0x5F, 0x13A, 0x60, 0x13B, 0x61, 0x13C,
0x62, 0x13D, 0x6000, YYOTHERS, 0x63, 0x137, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134,
0x6D, 0x135, 0x6E, 0x148, 0x2034, YYOTHERS, 0x6F, 0x137,
0x70, 0x13A, 0x71, 0x147, 0x72, 0x148, 0x73, 0x132,
0x6000, YYOTHERS, 0x74, 0x130, 0x201C, YYOTHERS, 0x201F, YYOTHERS,
0x75, 0x130, 0x200E, YYOTHERS, 0x76, 0x14C, 0x6000, YYOTHERS,
0x5A, 0x137, 0x5B, 0x13A, 0x5C, 0x13B, 0x2010, YYOTHERS,
0x5E, 0x137, 0x5F, 0x13A, 0x60, 0x13B, 0x61, 0x13C,
0x62, 0x13D, 0x2011, YYOTHERS, 0x63, 0x137, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134,
0x6D, 0x135, 0x77, 0x133, 0x2012, YYOTHERS, 0x6F, 0x137,
0x70, 0x13A, 0x71, 0x147, 0x72, 0x148, 0x73, 0x132,
0x2013, YYOTHERS, 0x2018, YYOTHERS, 0x78, 0x14C, 0x6000, YYOTHERS,
0x79, 0x130, 0x2015, YYOTHERS, 0x7A, 0x14C, 0x6000, YYOTHERS,
0x7B, 0x14C, 0x6000, YYOTHERS, 0x2007, YYOTHERS, 0x2006, YYOTHERS,
0x7C, 0x130, 0x6000, YYOTHERS, 0x11, 0x101, 0x12, 0x102,
0x13, 0x10B, 0x7D, 0x10C, 0x15, 0x13F, 0x16, 0x140,
0x17, 0x141, 0x18, 0x142, 0x19, 0x13B, 0x1B, 0x147,
0x1C, 0x148, 0x1D, 0x121, 0x1E, 0x143, 0x1F, 0x145,
0x20, 0x125, 0x21, 0x149, 0x22, 0x14A, 0x23, 0x14B,
0x24, 0x129, 0x25, 0x146, 0x6000, YYOTHERS, 0x2047, YYOTHERS,
0x2046, YYOTHERS, 0x2043, YYOTHERS, 0x63, 0x137, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134,
0x6D, 0x135, 0x6000, YYOTHERS, 0x2045, YYOTHERS, 0x2044, YYOTHERS,
0x5A, 0x137, 0x5B, 0x13A, 0x5C, 0x13B, 0x6000, YYOTHERS,
0x2067, YYOTHERS, 0x2053, YYOTHERS, 0x201D, YYOTHERS, 0x5A, 0x137,
0x5B, 0x13A, 0x5C, 0x13B, 0x7F, 0x11D, 0x6000, YYOTHERS,
0x5E, 0x137, 0x5F, 0x13A, 0x60, 0x13B, 0x61, 0x13C,
0x62, 0x13D, 0x80, 0x11D, 0x6000, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0x81, 0x11D, 0x6000, YYOTHERS,
0x6F, 0x137, 0x70, 0x13A, 0x71, 0x147, 0x82, 0x11D,
0x72, 0x148, 0x73, 0x132, 0x6000, YYOTHERS, 0x83, 0x130,
0x84, 0x11F, 0x6000, YYOTHERS, 0x85, 0x10C, 0x6000, YYOTHERS,
0x5A, 0x137, 0x5B, 0x13A, 0x5C, 0x13B, 0x2036, YYOTHERS,
0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139,
0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135, 0x2035, YYOTHERS,
0x2055, YYOTHERS, 0x2054, YYOTHERS, 0x2060, YYOTHERS, 0x205F, YYOTHERS,
0x12, 0x102, 0x52, 0x13B, 0x86, 0x147, 0x23, 0x14B,
0x6000, YYOTHERS, 0x12, 0x102, 0x52, 0x13B, 0x53, 0x147,
0x21, 0x149, 0x23, 0x14B, 0x25, 0x146, 0x6000, YYOTHERS,
0x2059, YYOTHERS, 0x2058, YYOTHERS, 0x2069, YYOTHERS, 0x2068, YYOTHERS,
0x200A, YYOTHERS, 0x8F, 0x10C, 0x90, 0x130, 0x6000, YYOTHERS,
0xA2, 0x10C, 0xA3, 0x130, 0x6000, YYOTHERS, 0xA4, 0x147,
0x21, 0x149, 0x25, 0x146, 0x6000, YYOTHERS, 0xA4, 0x147,
0x21, 0x149, 0x25, 0x146, 0x6000, YYOTHERS, 0x2009, YYOTHERS,
0x2003, YYOTHERS, 0x2004, YYOTHERS, 0x2008, YYOTHERS, 0x2021, YYOTHERS,
0x2020, YYOTHERS, 0x2038, YYOTHERS, 0x2062, YYOTHERS, 0x203D, YYOTHERS,
0x205B, YYOTHERS, 0x11, 0x101, 0x12, 0x102, 0x13, 0x10B,
0xAF, 0x10C, 0x15, 0x13F, 0x16, 0x140, 0x17, 0x141,
0x18, 0x142, 0x19, 0x13B, 0x1B, 0x147, 0x1E, 0x143,
0x1F, 0x145, 0x20, 0x125, 0x21, 0x149, 0x22, 0x14A,
0x23, 0x14B, 0x25, 0x146, 0x6000, YYOTHERS, 0x2024, YYOTHERS,
0xB1, 0x130, 0x6000, YYOTHERS, 0x5F, 0x13A, 0x60, 0x13B,
0x61, 0x13C, 0x62, 0x13D, 0x80, 0x11D, 0x6000, YYOTHERS,
0x70, 0x13A, 0x71, 0x147, 0x82, 0x11D, 0x6000, YYOTHERS,
0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139,
0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135, 0xB2, 0x11F,
0x6000, YYOTHERS, 0x5B, 0x13A, 0x5C, 0x13B, 0x203F, YYOTHERS,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x2039, YYOTHERS,
0x203E, YYOTHERS, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x203A, YYOTHERS, 0xB3, 0x130, 0xB4, 0x11F, 0x6000, YYOTHERS,
0xB5, 0x10C, 0x6000, YYOTHERS, 0x5F, 0x13A, 0x60, 0x13B,
0x61, 0x13C, 0x62, 0x13D, 0x2042, YYOTHERS, 0x61, 0x13C,
0x62, 0x13D, 0x2063, YYOTHERS, 0x61, 0x13C, 0x62, 0x13D,
0x2064, YYOTHERS, 0x2065, YYOTHERS, 0x2066, YYOTHERS, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x69, 0x138, 0x6A, 0x139, 0x2040, YYOTHERS, 0x203B, YYOTHERS,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x2049, YYOTHERS,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x204A, YYOTHERS,
0x204B, YYOTHERS, 0x204C, YYOTHERS, 0x204D, YYOTHERS, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x204E, YYOTHERS, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x204F, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x2050, YYOTHERS,
0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139,
0x6B, 0x136, 0x6D, 0x135, 0x2051, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x2052, YYOTHERS, 0xB6, 0x130, 0xB7, 0x11F, 0x6000, YYOTHERS,
0xB8, 0x10C, 0x6000, YYOTHERS, 0xA4, 0x147, 0x21, 0x149,
0x25, 0x146, 0x6000, YYOTHERS, 0x70, 0x13A, 0x71, 0x147,
0x2041, YYOTHERS, 0x71, 0x147, 0x205C, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0xB9, 0x130, 0xBA, 0x11D,
0x6000, YYOTHERS, 0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138,
0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135,
0xBB, 0x11F, 0x6000, YYOTHERS, 0x70, 0x13A, 0x71, 0x147,
0x2057, YYOTHERS, 0x201B, YYOTHERS, 0x200D, YYOTHERS, 0x2014, YYOTHERS,
0x2016, YYOTHERS, 0xBC, 0x14C, 0x6000, YYOTHERS, 0xBD, 0x130,
0xBE, 0x11F, 0x6000, YYOTHERS, 0x63, 0x137, 0x64, 0x13A,
0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E,
0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134,
0x6D, 0x135, 0xBF, 0x11F, 0x6000, YYOTHERS, 0x2023, YYOTHERS,
0x11, 0x101, 0x12, 0x102, 0x13, 0x10B, 0xC1, 0x10C,
0x15, 0x13F, 0x16, 0x140, 0x17, 0x141, 0x18, 0x142,
0x19, 0x13B, 0x1B, 0x147, 0x1E, 0x143, 0x1F, 0x145,
0x20, 0x125, 0x21, 0x149, 0x22, 0x14A, 0x23, 0x14B,
0x25, 0x146, 0x6000, YYOTHERS, 0x202E, YYOTHERS, 0xC3, 0x130,
0x6000, YYOTHERS, 0x11, 0x101, 0x12, 0x102, 0x13, 0x10B,
0xC4, 0x10C, 0x15, 0x13F, 0x16, 0x140, 0x17, 0x141,
0x18, 0x142, 0x19, 0x13B, 0x1B, 0x147, 0x1E, 0x143,
0x1F, 0x145, 0x20, 0x125, 0x21, 0x149, 0x22, 0x14A,
0x23, 0x14B, 0x25, 0x146, 0x6000, YYOTHERS, 0x2029, YYOTHERS,
0xC6, 0x130, 0x6000, YYOTHERS, 0x205E, YYOTHERS, 0x2056, YYOTHERS,
0x2005, YYOTHERS, 0x2027, YYOTHERS, 0x2025, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0xC9, 0x11F, 0x6000, YYOTHERS,
0xCA, 0x130, 0xCB, 0x11F, 0x6000, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0xCC, 0x11F, 0x6000, YYOTHERS,
0xCE, 0x130, 0xCF, 0x11F, 0x6000, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0xD0, 0x11F, 0x6000, YYOTHERS,
0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139,
0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135, 0xD2, 0x11D,
0x6000, YYOTHERS, 0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138,
0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135,
0xD3, 0x11F, 0x6000, YYOTHERS, 0x2026, YYOTHERS, 0x2031, YYOTHERS,
0x202F, YYOTHERS, 0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138,
0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135,
0xD5, 0x11F, 0x6000, YYOTHERS, 0x202C, YYOTHERS, 0x202A, YYOTHERS,
0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C,
0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139,
0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135, 0xD7, 0x11F,
0x6000, YYOTHERS, 0x205D, YYOTHERS, 0x2028, YYOTHERS, 0x63, 0x137,
0x64, 0x13A, 0x65, 0x13B, 0x66, 0x13C, 0x67, 0x13D,
0x68, 0x13E, 0x69, 0x138, 0x6A, 0x139, 0x6B, 0x136,
0x6C, 0x134, 0x6D, 0x135, 0xD8, 0x11F, 0x6000, YYOTHERS,
0x2030, YYOTHERS, 0x63, 0x137, 0x64, 0x13A, 0x65, 0x13B,
0x66, 0x13C, 0x67, 0x13D, 0x68, 0x13E, 0x69, 0x138,
0x6A, 0x139, 0x6B, 0x136, 0x6C, 0x134, 0x6D, 0x135,
0xD9, 0x11F, 0x6000, YYOTHERS, 0x202B, YYOTHERS, 0x2032, YYOTHERS,
0x202D, YYOTHERS 
};
#define YYNOCHAR (-1000)
#define	yyerrok	yyerrflag=0
#define	yyclearin	yylval=YYNOCHAR
int yystack[YYMAXDEPTH];
YYSTYPE yyvstack[YYMAXDEPTH], *yyv;
int yychar;

#ifdef YYDEBUG
int yydebug = 1;	/* No sir, not in the BSS */
#include <stdio.h>
#endif

short yyerrflag;
int *yys;

yyparse()
{
	register YYSTYPE *yypvt;
	int act;
	register unsigned *ip, yystate;
	int pno;
	yystate = 0;
	yychar = YYNOCHAR;
	yyv = &yyvstack[-1];
	yys = &yystack[-1];

stack:
	if( ++yys >= &yystack[YYMAXDEPTH] ) {
		write(2, "Stack overflow\n", 15);
		exit(1);
	}
	*yys = yystate;
	*++yyv = yyval;
#ifdef YYDEBUG
	if( yydebug )
		fprintf(stdout, "Stack state %d, char %d\n", yystate, yychar);
#endif

read:
	ip = &yyact[yypa[yystate]];
	if( ip[1] != YYNOCHAR ) {
		if( yychar == YYNOCHAR ) {
			yychar = yylex();
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stdout, "lex read char %d, val %d\n", yychar, yylval);
#endif
		}
		while (ip[1]!=YYNOCHAR) {
			if (ip[1]==yychar)
				break;
			ip += 2;
		}
	}
	act = ip[0];
	switch( act>>YYACTSH ) {
	case YYSHIFTACT:
		if( ip[1]==YYNOCHAR )
			goto YYerract;
		if( yychar != -1 )
			yychar = YYNOCHAR; /* dont throw away EOF */
		yystate = act&YYAMASK;
		yyval = yylval;
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "shift %d\n", yystate);
#endif
		if( yyerrflag )
			--yyerrflag;
		goto stack;

	case YYACCEPTACT:
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "accept\n");
#endif
		return(0);

	case YYERRACT:
	YYerract:
		switch (yyerrflag) {
		case 0:
			yyerror("Syntax error");

		case 1:
		case 2:

			yyerrflag = 3;
			while( yys >= & yystack[0] ) {
				ip = &yyact[yypa[*yys]];
				while( ip[1]!=YYNOCHAR )
					ip += 2;
				if( (*ip&~YYAMASK) == (YYSHIFTACT<<YYACTSH) ) {
					yystate = *ip&YYAMASK;
					goto stack;
				}
#ifdef YYDEBUG
				if( yydebug )
					fprintf(stderr, "error recovery leaves state %d, uncovers %d\n", *yys, yys[-1]);
#endif
				yys--;
				yyv--;
			}
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stderr, "no shift on error; abort\n");
#endif
			return(1);

		case 3:
#ifdef YYDEBUG
			if( yydebug )
				fprintf(stderr, "Error recovery clobbers char %o\n", yychar);
#endif
			if( yychar==YYEOFVAL )
				return(1);
			yychar = YYNOCHAR;
			goto read;
		}

	case YYREDACT:
		pno = act&YYAMASK;
#ifdef YYDEBUG
		if( yydebug )
			fprintf(stdout, "reduce %d\n", pno);
#endif
		yypvt = yyv;
		yyv -= yypn[pno];
		yys -= yypn[pno];
		yyval = yyv[1];
		switch(pno) {

case 3: {

#line 351 "asm.y"
 /* assembler command with parms */
		docmd(yypvt[-3].p, yypvt[-2].o, yypvt[-1].p); }break;

case 4: {

#line 353 "asm.y"
 /* a command with string parms */
		docmd(yypvt[-3].p, yypvt[-2].o, yypvt[-1].p); }break;

case 5: {

#line 355 "asm.y"
 /* command with a name & an expr */
		ecmd(yypvt[-5].p, yypvt[-4].o, yypvt[-3].p, yypvt[-1].d); }break;

case 6: {

#line 357 "asm.y"

		ecmd(yypvt[-2].p, yypvt[-1].o, NULL, 0L); }break;

case 7: {

#line 359 "asm.y"
	/* assembler command takes no parms */
		docmd(yypvt[-2].p, yypvt[-1].o, (parm *)NULL); }break;

case 8: {

#line 361 "asm.y"
 /* assembler command with data parm */
		ncmd(yypvt[-3].p, yypvt[-2].o, yypvt[-1].d); }break;

case 9: {

#line 363 "asm.y"
 /* data list */
		dcmd(yypvt[-3].p, yypvt[-2].o, yypvt[-1].d); }break;

case 10: {

#line 365 "asm.y"
	/* opcode operands */
		buildind(yypvt[-3].p, yypvt[-2].o, yypvt[-1].e); }break;

case 11: {

#line 367 "asm.y"
	/* label alone on line */
		buildlab(yypvt[-1].p); }break;

case 12: {

#line 369 "asm.y"
	/* syntax error */
		if (bcnt > 0)
			unmatched('[');
		if (bcnt < 0)
			unmatched(']');
		if (pcnt > 0)
			unmatched('(');
		if (pcnt < 0)
			unmatched(')');
		yyerrok; }break;

case 13: {

#line 380 "asm.y"

		yyval.d = yypvt[-2].d;
		yyval.d->next = yypvt[0].d; }break;

case 14: {

#line 383 "asm.y"

		yyval.d = yypvt[0].d; }break;

case 15: {

#line 385 "asm.y"

		yyval.d = NULL; }break;

case 16: {

#line 388 "asm.y"

		yyval.d = gitem('y');
		yyval.d->d.y  = yypvt[0].s; }break;

case 17: {

#line 391 "asm.y"

		yyval.d = gitem('d');
		yyval.d->d.d = yypvt[0].dbl; }break;

case 18: {

#line 394 "asm.y"

		yyval.d = gitem('l');
		yyval.d->d.l = yypvt[0].val; }break;

case 19: {

#line 397 "asm.y"

		yyval.d = gitem('s');
		yyval.d->d.s = yypvt[0].t; }break;

case 20: {

#line 400 "asm.y"

		yyval.d = yypvt[0].d;
		yyval.d->count = yypvt[-2].val; }break;

case 21: {

#line 404 "asm.y"
		/* start parm list */
		yyval.p = yypvt[0].p; }break;

case 22: {

#line 406 "asm.y"
	/* chain parm list */
		yyval.p = yypvt[-2].p;
		yyval.p->next = yypvt[0].p; }break;

case 23: {

#line 409 "asm.y"

		yyval.p = NULL; }break;

case 24: {

#line 412 "asm.y"
		/* start a parm */
		yyval.p = (parm *)gcpy(yypvt[0].t, offset(parm, str)); }break;

case 25: {

#line 415 "asm.y"
 
		yyval.p = NULL; }break;

case 26: {

#line 417 "asm.y"

		yyval.p = (parm *)gcpy(yypvt[0].t, offset(parm, str)); }break;

case 27: {

#line 421 "asm.y"

		yyval.e = yypvt[-2].e;
		yyval.e->next = yypvt[0].e; }break;

case 28: {

#line 424 "asm.y"

		yyval.e = yypvt[0].e; }break;

case 29: {

#line 426 "asm.y"

		lflags |= A_INDIR;
		yyval.e = yypvt[0].e; }break;

case 30: {

#line 429 "asm.y"

		yyval.e = NULL; }break;

case 31: {

#line 432 "asm.y"

	yyval.e->sg = -1;
	yyval.e = yypvt[0].e; }break;

case 32: {

#line 435 "asm.y"

		yyval.e = yypvt[0].e;
		if (yypvt[-2].s->flag != SEG_REG)
			regerror(yypvt[-2].s);
		yyval.e->sg = yypvt[-2].s->loc; }break;

case 33: {

#line 442 "asm.y"

		yyval.e = qbild(0L, T_R, yypvt[0].s, NULL, 0L, NULL); }break;

case 34: {

#line 445 "asm.y"

     		yyval.e = fbild(0L);	}break;

case 35: {

#line 448 "asm.y"

     		yyval.e = fbild(yypvt[-1].val); }break;

case 36: {

#line 451 "asm.y"

		yyval.e = qbild(0L, T_RI, yypvt[-1].s, NULL, 0L, NULL); }break;

case 37: {

#line 454 "asm.y"

		yyval.e = qbild(0L, T_RIS, yypvt[-3].s, NULL, yypvt[-1].val, NULL); }break;

case 38: {

#line 457 "asm.y"

		yyval.e = qbild(0L, T_RIS, yypvt[-3].s, NULL, yypvt[-1].val, NULL); }break;

case 39: {

#line 460 "asm.y"

     		yyval.e = qbild(0L, T_RIX, yypvt[-3].s, yypvt[-1].s, 0L, NULL); }break;

case 40: {

#line 463 "asm.y"

     		yyval.e = qbild(0L, T_RIXS, yypvt[-5].s, yypvt[-3].s, yypvt[-1].val, NULL); }break;

case 41: {

#line 466 "asm.y"

		yyval.e = qbild(yypvt[-3].val, T_RID, yypvt[-1].s, NULL, 0L, NULL); }break;

case 42: {

#line 469 "asm.y"

		yyval.e = qbild(yypvt[-5].val, T_RIDS, yypvt[-3].s, NULL, yypvt[-1].val, NULL); }break;

case 43: {

#line 472 "asm.y"

		yyval.e = qbild(yypvt[-6].val, T_RIDS, yypvt[-3].s, NULL, yypvt[-1].val, NULL); }break;

case 44: {

#line 475 "asm.y"

     		yyval.e = qbild(yypvt[-5].val, T_RIXD, yypvt[-3].s, yypvt[-1].s, 0L, NULL); }break;

case 45: {

#line 478 "asm.y"

     		yyval.e = qbild(yypvt[-7].val, T_RIXDS, yypvt[-5].s, yypvt[-3].s, yypvt[-1].val, NULL); }break;

case 46: {

#line 481 "asm.y"

		yyval.e = qbild(yypvt[-3].s->loc, T_RID, yypvt[-1].s, NULL, 0L, yypvt[-3].s); }break;

case 47: {

#line 484 "asm.y"

		yyval.e = qbild(yypvt[-5].s->loc, T_RIDS, yypvt[-3].s, NULL, yypvt[-1].val, yypvt[-5].s); }break;

case 48: {

#line 487 "asm.y"

		yyval.e = qbild(yypvt[-6].s->loc, T_RIDS, yypvt[-3].s, NULL, yypvt[-1].val, yypvt[-6].s); }break;

case 49: {

#line 490 "asm.y"

     		yyval.e = qbild(yypvt[-5].s->loc, T_RIXD, yypvt[-3].s, yypvt[-1].s, 0L, yypvt[-5].s); }break;

case 50: {

#line 493 "asm.y"

     		yyval.e = qbild(yypvt[-7].s->loc, T_RIXDS, yypvt[-5].s, yypvt[-3].s, yypvt[-1].val, yypvt[-7].s); }break;

case 51: {

#line 496 "asm.y"

     		yyval.e = qbild(yypvt[0].s->loc, T_D, NULL, NULL, 0L, yypvt[0].s); }break;

case 52: {

#line 499 "asm.y"

     		yyval.e = qbild(yypvt[0].val, T_D, NULL, NULL, 0L, NULL); }break;

case 53: {

#line 502 "asm.y"

		yyval.e = setImm(yypvt[0].val, (sym *)NULL); }break;

case 54: {

#line 505 "asm.y"

		yyval.e = setImm(yypvt[0].s->loc, yypvt[0].s); }break;

case 55: {

#line 511 "asm.y"
 yyval.s = yypvt[0].s; }break;

case 56: {

#line 512 "asm.y"
 yyval.s=yypvt[-1].s; }break;

case 57: {

#line 513 "asm.y"
 yyval.s = copySym(yypvt[-2].s); yyval.s->loc += yypvt[0].val; }break;

case 58: {

#line 514 "asm.y"
 yyval.s = copySym(yypvt[-2].s); yyval.s->loc -= yypvt[0].val; }break;

case 59: {

#line 515 "asm.y"
 yyval.s = copySym(yypvt[0].s); yyval.s->loc += yypvt[-2].val; }break;

case 60: {

#line 518 "asm.y"

		yyval.val = yypvt[0].val; }break;

case 61: {

#line 520 "asm.y"

		yyval.val = yypvt[-1].val; }break;

case 62: {

#line 523 "asm.y"

		ckseg(yypvt[-2].s, yypvt[0].s); yyval.val = yypvt[-2].s->loc - yypvt[0].s->loc; }break;

case 63: {

#line 526 "asm.y"

		ckseg(yypvt[-2].s, yypvt[0].s); yyval.val = compare((int)yypvt[-1].val, yypvt[-2].s->loc - yypvt[0].s->loc); }break;

case 64: {

#line 528 "asm.y"

		yyval.val = compare((int)yypvt[-1].val, yypvt[-2].val - yypvt[0].val); }break;

case 65: {

#line 530 "asm.y"

		yyval.val = compare((int)yypvt[-1].val, (long)strcmp(yypvt[-2].t, yypvt[0].t)); }break;

case 66: {

#line 532 "asm.y"

		yyval.val = fcompare((int)yypvt[-1].val, yypvt[-2].dbl - yypvt[0].dbl); }break;

case 67: {

#line 535 "asm.y"

		yyval.val = yypvt[0].s->size; }break;

case 68: {

#line 538 "asm.y"

		yyval.val = yypvt[0].s->loc; }break;

case 69: {

#line 541 "asm.y"

		yyval.val = yypvt[0].s->sg + 1; }break;

case 70: {

#line 544 "asm.y"

		yyval.val = yypvt[0].s->statement && (statement >= yypvt[0].s->statement); }break;

case 71: {

#line 546 "asm.y"

		yyval.val = 1; }break;

case 72: {

#line 548 "asm.y"

		yyval.val = parmCt(); }break;

case 73: {

#line 551 "asm.y"

		yyval.val = yypvt[-2].val + yypvt[0].val; }break;

case 74: {

#line 553 "asm.y"

		yyval.val = yypvt[-2].val - yypvt[0].val; }break;

case 75: {

#line 555 "asm.y"

		yyval.val = yypvt[-2].val * yypvt[0].val; }break;

case 76: {

#line 557 "asm.y"

		yyval.val = yypvt[-2].val / yypvt[0].val; }break;

case 77: {

#line 559 "asm.y"

		yyval.val = yypvt[-2].val % yypvt[0].val; }break;

case 78: {

#line 562 "asm.y"

		yyval.val = yypvt[-2].val << yypvt[0].val; }break;

case 79: {

#line 564 "asm.y"

		yyval.val = yypvt[-2].val >> yypvt[0].val; }break;

case 80: {

#line 568 "asm.y"

		yyval.val = yypvt[-2].val & yypvt[0].val; }break;

case 81: {

#line 570 "asm.y"

		yyval.val = yypvt[-2].val | yypvt[0].val; }break;

case 82: {

#line 572 "asm.y"

		yyval.val = yypvt[-2].val ^ yypvt[0].val; }break;

case 83: {

#line 574 "asm.y"

		yyval.val = - yypvt[0].val; }break;

case 84: {

#line 576 "asm.y"

		yyval.val = !yypvt[0].val; }break;

case 85: {

#line 578 "asm.y"

		yyval.val = ~yypvt[0].val; }break;

case 86: {

#line 581 "asm.y"

		yyval.val = (yypvt[-1].val > strlen(yypvt[-3].t)) ? 0 : yypvt[-3].t[(short)yypvt[-1].val]; }break;

case 87: {

#line 583 "asm.y"

		yyval.val = stringSearch(yypvt[-2].t, yypvt[0].t); }break;

case 88: {

#line 585 "asm.y"

		yyval.val = atol(yypvt[0].t); }break;

case 89: {

#line 587 "asm.y"

		yyval.val = yypvt[0].dbl; }break;

case 90: {

#line 590 "asm.y"

		yyval.t = gcpy(yypvt[0].t, 0); }break;

case 91: {

#line 592 "asm.y"

		yyval.t = yypvt[-1].t; }break;

case 92: {

#line 594 "asm.y"

		yyval.t = concat(yypvt[-2].t, yypvt[0].t); }break;

case 93: {

#line 596 "asm.y"

		yyval.t = substr(yypvt[-5].t, yypvt[-3].val, yypvt[-1].val); }break;

case 94: {

#line 598 "asm.y"

		yyval.t = substr(yypvt[-3].t, yypvt[-1].val, strlen(yypvt[-3].t) - yypvt[-1].val); }break;

case 95: {

#line 600 "asm.y"

		yyval.t = galloc(12);
		sprintf(yyval.t, "%ld", yypvt[0].val); }break;

case 96: {

#line 603 "asm.y"

		yyval.t = galloc(20);
		sprintf(yyval.t, "%g", yypvt[0].dbl); }break;

case 97: {

#line 608 "asm.y"

		yyval.dbl = yypvt[0].dbl; }break;

case 98: {

#line 610 "asm.y"

		yyval.dbl = yypvt[-1].dbl; }break;

case 99: {

#line 612 "asm.y"

		yyval.dbl = yypvt[-2].dbl + yypvt[0].dbl; }break;

case 100: {

#line 614 "asm.y"

		yyval.dbl = yypvt[-2].dbl - yypvt[0].dbl; }break;

case 101: {

#line 616 "asm.y"

		yyval.dbl = yypvt[-2].dbl * yypvt[0].dbl; }break;

case 102: {

#line 618 "asm.y"

		yyval.dbl = yypvt[-2].dbl / yypvt[0].dbl; }break;

case 103: {

#line 620 "asm.y"

		yyval.dbl = - yypvt[0].dbl; }break;

case 104: {

#line 622 "asm.y"

		yyval.dbl = strtod(yypvt[0].t, (char **)NULL); }break;

case 105: {

#line 624 "asm.y"

		yyval.dbl = yypvt[0].val; }break;

		}
		ip = &yygo[ yypgo[yypdnt[pno]] ];
		while( *ip!=*yys && *ip!=YYNOCHAR )
			ip += 2;
		yystate = ip[1];
		goto stack;
	}
}




