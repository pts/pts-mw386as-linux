
#line 259 "asm.y"
typedef union {
	long	val;	/* numeric value */
	double  dbl;
	sym	*s;	/* name size loc bitd bitl flag */
	opc	*o;	/* opcode kind */
	expr	*e;	/* mode loc size exp bitd bitl chain next */
	char	*t;	/* token */
	parm	*p;	/* parm  */
	data	*d;	/* data item */
	} YYSTYPE;
#define PC 256
#define NUMBER 257
#define FNUM 258
#define COMPARISON 311
#define OP 260
#define DATA 261
#define CMD 262
#define DCMD 263
#define ICMD 264
#define NCMD 265
#define ECMD 266
#define IDENTIFIER 267
#define REG 268
#define DEFINED 319
#define SIZEOF 320
#define SEGMENT 321
#define LOCATION 322
#define PLUS 314
#define MINUS 315
#define TIMES 316
#define DIVIDE 317
#define REM 318
#define LSHIFT 312
#define RSHIFT 313
#define AND 310
#define OR 308
#define XOR 309
#define COMMA 304
#define LPAREN 327
#define RPAREN 285
#define LBRACK 328
#define RBRACK 287
#define AT 306
#define D_SIGN 289
#define NOT 323
#define BANG 325
#define COLON 305
#define PARMCT 293
#define TOSTRING 329
#define TONUMBER 330
#define TOFLOAT 331
#define FSTACK 297
#define NL 332
#define TOKEN 326
#define P_SIGN 307
#define LEN 324
#ifdef YYTNAMES
extern readonly struct yytname
{
	char	*tn_name;
	int	tn_val;
} yytnames[];
#endif
extern	YYSTYPE	yylval;
