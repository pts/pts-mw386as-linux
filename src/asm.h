/*
 * 80386 assembler header.
 */
#include <stdio.h>

#include "intsize.h"

#define new(s) ((s *)alloc(sizeof(s)))
#define gnew(s) ((s *)galloc(sizeof(s)))
#define clear(x) memset((char *)(x), 0, sizeof(*(x)))
#define offset(st, memb) ((size_t)(((st *)0)->memb))

typedef struct expr expr;	/* generated address stuff */
typedef struct inpctl inpctl;	/* include level of control */
typedef struct macctl macctl;	/* macro level of control */
typedef struct macline macline;	/* a macro line */
typedef struct macro macro;	/* macro symbol */
typedef struct opc opc;		/* opcode symbol entry */
typedef struct parm parm;	/* a parameter on a control line */
typedef struct data data;	/* data block for dc.w dc.d etc */
typedef struct sym sym;		/* allocated  symbol table entry */
typedef struct symt  symt;	/* opcode table entry */
typedef struct nhash nhash;	/* hash table entry */
typedef struct psym psym;	/* predefined symbol table entry */
typedef struct symdef symdef;	/* defined in c_out.c uses SYMENT & AUXENT */

struct expr {	/* generated 80386 address stuff */
	expr *next;
	char mode;	/* mode byte */
	i32_t exp;	/* displacment */
	short sg;	/* segment override */
	psym *r1;	/* first register */
	psym *r2;	/* second register */
	char scale;	/* scaling factor */
	i32_t size;	/* size of target */
	sym	*ref;	/* pointer to associated symbol */
};

#define T_IMM 	1	/* $ immediate */
#define T_R	2	/* reg */
#define T_RI	3	/* ( reg ) */
#define T_RID	4	/* disp ( reg ) */
#define T_RIX	5	/* ( reg, reg ) */
#define T_RIXD  6	/* disp ( reg, reg ) */
#define T_RIS	7	/* ( reg, scale ) */
#define T_RIDS	8 	/* disp ( reg, scale ) */
#define T_RIXS  9	/* ( reg , reg, scale ) */
#define T_RIXDS 10	/* disp ( reg , reg, scale ) */
#define T_D	11	/* displacment */
#define T_FP	12	/* %st( n ) */

struct opc {	/* slice in pref tab */
	short code;	/* opcode */
	short kind;	/* index to symt table */
};

struct symt {	/* Opcode symbol type. */
	unsigned short type;		/* yacc type */
	unsigned short bldr;		/* code for builder */
	unsigned char operands;		/* operands for this kind of symbol */
	unsigned char ap[3];		/* operand types */
};

struct nhash {	/* name hash entry */
	short next;			/* index to next symbol same hash */
	short nameIx;			/* index into charLump */
	char  nlen;			/* length of name */
	char  count;			/* match count in prefTab */
	short prefIx;			/* index into prefTab */
};

struct psym {	/* predefined symbol table entry */
	psym	*next;			/* same symbol with same hash */
	short	type;			/* yacc type or coff debug type */
	i32_t	loc;			/* location of coff debug value */
	i32_t	size;			/* length */
	short	sg;			/* segment plus 1 */
	short	flag;			/* flag bits */
	sym 	*ref;			/* base copy of this symbol on symtab */
	short	num;			/* symbol number for relocation */
	int	statement;		/* statement no of definition */
	char	*name;			/* symbol name */
};

struct sym {	/* allocated symbol table entry */
	psym	*next;			/* same symbol with same hash */
	short	type;			/* yacc type */
	i32_t	loc;			/* location */
	i32_t	size;			/* length */
	short	sg;			/* segment plus 1 */
	short	flag;			/* flag bits */
	sym 	*ref;			/* base copy of this symbol on symtab */
	short	num;			/* symbol number for relocation */
	int	statement;		/* statement no of definition */
	char	name[1];		/* symbol name */
};

/* Register flags */
#define ORD_REG	0
#define	SEG_REG	0x0800
#define CTL_REG	0x0400
#define DEB_REG	0x0200
#define TST_REG	0x0100

#define BAD	1		/* Bad exit() code */
#define	NCPLN	16		/* max external symbol length */
#define NINPUT	256		/* Input line size */

/* Listing */
#define NLIST	0		/* No listing */
#define SLIST	1		/* Source only */
#define ALIST	2		/* Address only */
#define	BLIST	3		/* Byte listing */

#define	dot	(symtab[0])	/* Dot, current loc */

/* symbol type flags */
#define S_EXDEF 0x01	/* defined here visable elsewhere */
#define S_LOCAL 0x02	/* defined here */
#define S_EXREF 0x04	/* defined elsewhere visable here */
#define S_UNDEF 0x08	/* unidentified so far */
#define S_DUPSYM 0x10	/* duplicate symbol */
#define S_XSYM  0x20	/* non identifier */
#define S_USED  0x40	/* symbol used */
#define S_COMMON 0x80	/* common symbol */

#define S_ASYM	0x3F	/* allocated symbol */

#define SYMNAME(sp) ((sp->flag&S_ASYM) ? ((sym *)sp)->name : ((psym *)sp)->name)

struct parm {	/* a parameter on a control line */
	parm *next;	/* next parm on line */
	char str[1];
};

struct macline {	 /* a macro line */
	macline *next;
	char line[1];	/* data of indefinate length */
};

struct macro {	/* macro symbol */
	macro	*next;	/* macro's searched sequentially */
	unsigned short type;	/* same position as sym type */
	macline *first;	/* first line of macro */
	parm	*names; /* names of macro parms */
	char name[1];	/* name of macro */
};

#define WHILETYPE 0	/* while loop */
#define MACTYPE 1	/* macro */
#define MACSTR  2	/* string */
#define MACSCAN 3	/* string being scaned */

struct macctl {	 /* macro level of control */
	macctl  *prev;	/* previous level of control */
	unsigned short type; /* do mac while etc */
	parm	*parms;	/* macro parm list */
	parm	*names;	/* macro parm names */
	macline *first; /* where it started for do else etc */
	macline *curr;	/* this macro line */
	short	expno;	/* this macro expansion number */
};

/* macctl types */
#define INMACDEF 1	/* in macro definition */
#define INWDEF	 2	/* finding limits of a while */
#define INPWDEF	 3	/* defining while internal to another while or mac */
#define PMACDEF  4	/* macro definition inside macro definition */
#define INIF0	 5	/* in unsuccesfull if */
#define INIFX	 6	/* if in unsuccessfull if */
#define INIF1	 7	/* in successfull if */
#define INWHILE	 8	/* in successfull while */
#define NORMAL	 9	/* base mode */

struct inpctl {		/* include level of control */
	inpctl *prev;		/* previous file */
	FILE   *fp;		/* File ptr */
	short	lineNo;		/* current line number */
	char	name[1];	/* file name */
};

struct data {	/* misc data item */
	data *next;
	unsigned short type;	/* 'd', 'l', 'y', 's' */
	unsigned short count;	/* repeat count */
	union d {
		double	d;	/* double data */
		i32_t	l;	/* i32_t data */
		sym	*y;	/* pointer to original symbol */
		char	*s;	/* pointer to string */
	} d;
};

/* bits for lflags */
#define A_SHORT 1	/* Uses short address */
#define A_LONG  2	/* Uses i32_t address */
#define O_SHORT 4	/* Uses short operand */
#define O_LONG  8	/* Uses i32_t operand */
#define A_INDIR 16	/* Uses indirect operand */

/*
 * Some opcodes need to be known outside the tables.
 * the following defines at least give them names.
 */
#define PREFIX_OP 0x66	/* Operand size prefix */
#define PREFIX_AD 0x67	/* Address mode prefix */
#define PREFIX_ES 0x26	/* Use %es: */
#define PREFIX_CS 0x2e	/* Use %cs: */
#define PREFIX_SS 0x36	/* Use %ss: */
#define PREFIX_DS 0x3e	/* Use %ds: */
#define PREFIX_FS 0x64	/* Use %ds: */
#define PREFIX_GS 0x65	/* Use %ds: */

#define JMP_SHORT 0xEB 
#define JMP_NEAR  0xE9
#define JMP_INDIR 0xFF04
#define CALL_NEAR 0xE8
#define CALL_INDIR 0xFF02
#define JCC_SHORT 0x70
#define JCC_NEAR  0x0F80

#define MOV_BYTE  0xC6	/* Ambigous move op code */
#define INSB	  0x6C
#define INSW	  0x6D
#define NOP	  0x90
#define POPA	  0x61
#define AAM	  ((unsigned short)0xD40A)
#define POP_MEM	  ((unsigned short)0x8F00)
#define NON_OP	  ((unsigned short)0x0201)	/* not an op code */
#include "asme.h"
