/*
 * Build code tables for 80386 assembler.
 * Also build .h file and assembler test file.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asflags.h"
#include "intsize.h"
#include "utype.h"

char *newcpy(register char *s);
i32_t randl(void);
char *alloc(size_t n);
extern char *as_optarg;
int getargs(int argc, char * const argv[], const char *optstring);  /* getopt(3) would also work, but that's not C89. */
unsigned short hash(const char *p);
FILE *xopen(const char *fn, const char *acs);
char *as_getline(FILE *ifp, int *lineno);

typedef struct opts opts;
typedef struct funs funs;
typedef struct regs regs;
typedef struct oper oper;

char *allntab;		/* all names table */
short allnct, allnlen;

short *htab;		/* hash table */

struct oper {	/* operand types for test builder */
	char *name;
	int  base;		/* != 0 if equate must be built for this type */
	short flag;		/* flag bits */
	unsigned goodct;	/* number of good items on this list */
	char *goodlist;		/* valid productions for this type */
	unsigned badct;		/* number of bad items on this list */
	char *badlist;		/* bad productions to test error checking */
} *opertab;
short operct, operlen;

/* oper flags */
#define X_LARGE	1	/* contains 386 mode stuff */
#define X_SMALL 2	/* contains 286 mode stuff */

struct opts {		/* opcode table */
	char *name;
	short opcode;
	short hash;
	short gen;		/* index into gentab */
	short pt;		/* index into alltab */
	short len;		/* length in alltab */
	short fun;		/* index into fun */
	short count;		/* count of items with this name */
	short lineno;		/* line number on source table */
} *optab;
short opct, oplen;

struct funs {
	char *name;
	const char *type;	/* yacc type */
	short opt;	/* generation type */
	char operands;  /* operand ct */
	char ap[3];	/* operands */
} *funtab;
short funct, funlen;

struct regs {
	char *name;
	const char *ytype;
	short loc;
	short len;
} *regtab;
short regct, reglen;

#define START(n, m, type) n##tab = (type*)alloc((n##len = m) * sizeof(*n##tab))

/* expander for tables */
#define EXPAND(n, type) if((n##len <= (++n##ct)) \
 && (NULL == (n##tab = (type*)realloc(n##tab, ((n##len += 10) * sizeof(*n##tab)))))) \
	outSpace(__LINE__)

/* expander for allnames */
#define NEWN(n, r) if((n##len <= (n##ct += r)) \
 && (NULL == (n##tab = (char*)realloc(n##tab, n##len += 10)))) \
	outSpace(__LINE__)

extern char *comment;	/* from as_getline() */

static FILE *ofp, *ohp, *otp, *odp;	/* output files */
static char *line;	/* input line */
static int lineno = 1;	/* line number */
static int state;	/* opcodes, registers commands */
static int curgen;	/* index to current general name */
static unsigned nameCt;	/* name count */
static int lastp;	/* last entry on prefTab */
static short ct, opcode, opt;
static u32_t optDoc;
static char fname[22], opc[22], op1[22], op2[22], op3[22], cmd[22], yt[22];
static char curoperGen[22];
static int errors;		/* error count */

/* test selector switches */
static unsigned tmask = 0;		/* reject any not on this mask */
static unsigned nmask = 0;		/* take any on this mask */
static unsigned lswitch;		/* do large ops only */
static unsigned sswitch;		/* do small ops only */
static unsigned bswitch;		/* produce error test */
static unsigned dswitch;		/* don't generate document */
static unsigned eswitch;		/* don't generate test.s */

void showStats(int n);
void opBld(void);
static int produce(char *n);
int findOpr(char *name);

/*
 * Report error.
 */
static void error(const char *fmt, ...)
{
	va_list ap;
	fprintf(stderr, "%d: ", lineno);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	errors++;
}

/*
 * Out of space. or normal end.
 */
void outSpace(int line)
{
	if (line)
		fprintf(stderr, "Out of space at %d\n", line);
	showStats(1);
}

/*
 * Show generation statistics and exit.
 */
void showStats(int n)
{
	fprintf(stderr, "opct = %d, funct = %d, lineno = %d, ",
		opct, funct, lineno);
	fprintf(stderr, "regct = %d, operct = %d, allnct = %d\n",
		regct, operct, allnct);
	exit(n);
}

/*
 * Build test alternatives.
 * Line starts with [.bB] for base type [!eE] for extra type.
 * Caps for 386 productions, lower for 286 productions, punctuation for mixed.
 * Base types are the operand types actually on opcodes.
 * ? delimits bad choices.
 */
void
buildTst(void)
{
	register struct oper *curoper;
	register char *p;
	static int base;
	int state, c;

	EXPAND(oper, oper);
	curoper = opertab + operct - 1;

	switch (line[0]) {	/* mark base and extra */
	case '.':
		curoper->base = ++base;
		/* fallthrough */
	case '!':
		curoper->flag = (X_LARGE|X_SMALL);
		break;
	case 'B':
		curoper->base = ++base;
		/* fallthrough */
	case 'E':
		curoper->flag = X_LARGE;
		break;
	case 'b':
		curoper->base = ++base;
		/* fallthrough */
	case 'e':
		curoper->flag = X_SMALL;
		break;
	default:
		operct--;
		error("Bad test line");
		return;
	}

	for (p = line + 2; (c = *p) && !isspace(c); p++)
		;
	*p++ = '\0';
	curoper->name = newcpy(line + 2);

	/* Count the valid productions on the line */
	curoper->goodlist = p;
	for (state = 0; (c = *p); p++) {
		if (state) {	/* in a production */
			if (!isspace(c)) {
				if ('?' == c)
					break;
				continue;
			}
			state = 0;
			continue;
		}
		if (isspace(c))
			continue;
		if ('?' == c)
			break;
		curoper->goodct++;
		state = 1;
	}
	*p++ = '\0';
	curoper->goodlist = newcpy(curoper->goodlist);
	if (!c)
		return;

	/* count the invalid productions on the line */
	curoper->badlist = p;
	for (state = curoper->badct = 0; (c = *p); p++) {
		if (state) {
			if (!isspace(c))
				continue;
			state = 0;
			continue;
		}
		if (isspace(c))
			continue;
		curoper->badct++;
		state = 1;
	}
	curoper->badlist = newcpy(curoper->badlist);
}

/* A simplified sscanf(3) implementation. */
static void tabbld_sscanf(const char *s, const char *fmt, ...) {
  char c, *endp;
  va_list ap;
  va_start(ap, fmt);
  while ((c = *fmt++) != '\0') {
    if (c == ' ') {
      for (; isspace(*s); ++s) {}
    } else if (c == '%') {
      if ((c = *fmt++) == 'd' || c == 'x') {
        va_arg(ap, int*)[0] = strtol(s, &endp, c == 'd' ? 10 : 16);  /* For simplicity, we don't check for errors, and we allow negative. */
        s = endp;
      } else if (c == 's') {
        endp = va_arg(ap, char*);
        for (; *s != '\0' && !isspace(*s); *endp++ = *s++) {}
        *endp = '\0';
      } else {
        error("unsupported sscanf format specifier: %c", c); exit(2);
      }
    } else {
      if (c != *s) { error("sscanf expected character %c, got %c", c, *s); exit(2); }
      ++s;
    }
  }
#if 0  /* Not always true. */
  for (; isspace(*s); ++s) {}
  if (*s != '\0') { error("sscanf input remaining: %s", s); exit(2); }
#endif
}

/*
 * Build assembler directives.
 */
void buildDir(void)
{
	register funs *f;
	register opts *curoper;
	int i, j;

	tabbld_sscanf(line, "%d %s %s %s", &i, opc, cmd, yt);
	opcode = i;
	sprintf(fname, "S_%s", cmd);

	for (j = 0; j < funct; j++)
		if (!strcmp(funtab[j].name, fname))
			break;

	if (j == funct) {	/* not found build one */
		EXPAND(fun, funs);
		f = funtab + j;
		f->name = newcpy(fname);
		f->opt = f->operands = 0;
		f->type = newcpy(yt);
	}


	if (!strcmp(opc, "-"))	/* no opcode generated */
		return;

	i = opct;
	EXPAND(op, opts);
	curoper = optab + i;
	curoper->name = newcpy(opc);
	curoper->fun = j;
	curoper->opcode = opcode;
	curoper->gen = -1;
	curoper->hash = -2;
	curoper->lineno = lineno;
}

/*
 * Build register name entrys.
 */
void
buildReg(void)
{
	register regs *newregs;
	char name[22], ytype[22];
	int i, j;

	EXPAND(reg, regs);
	newregs = regtab + regct - 1;

	tabbld_sscanf(line,
		 "%s %s %d %d",
		 name, ytype, &i, &j);
	newregs->loc = i;
	newregs->len = j;
	newregs->name  = newcpy(name);
	newregs->ytype = newcpy(ytype);
}

/*
 * Read and preprocess opcode
 */
void
buildOp(void)
{
	register opts *curoper;
	char optf[8], *p;
	unsigned u;

	optDoc = opc[0] = op1[0] = op2[0] = op3[0] = '\0';
	tabbld_sscanf(line, "%s %s", optf, opc);

	if ('G' == optf[0]) {	/* general opcode */
		curgen = -1;
		if(opc[0]) {
			for (curgen = 0; curgen < opct; curgen++) {
				if (!strcmp(opc, optab[curgen].name)) {
					error("Dup Gen %s", opc);
					return;
				}
			}
			strcpy(curoperGen, opc);

			if (odp) fprintf(odp, "%s %d 0!", opc, lineno);
			if (odp) fprintf(odp, "\t\\fB%s\\fR\t\t%s\n",
				 opc, comment);

			EXPAND(op, opts);
			curoper = optab + curgen;
			curoper->name = newcpy(opc);
			curoper->fun = -1;
			curoper->hash = -1;
			curoper->gen = curgen; /* self pointing */
			curoper->lineno = lineno;
		}
		return;
	}			

	if (!opc[0])
		error("Null name");

	tabbld_sscanf(line, "%s %x %s %s %s %s", optf, &u, opc, op1, op2, op3);
	opcode = u;

	for (p = optf; ; p++) {
		switch(*p) {
		case 0:
		case '-':
			opt = optDoc;
			opBld();		return;
		case 'A':
			optDoc |= AMBIG_MATCH;	break;
		case 'i':
			optDoc |= INDEF_JMP;	break;
		case 'w':
			optDoc |= WORD_MODE;	break;
		case 'd':
			optDoc |= LONG_MODE;	break;
		case 'F':
			optDoc |= FLOAT_ESC;	break;
		case 'f':
			optDoc |= FLOAT_PFX;	break;
		case 'p':
			optDoc |= PFX_0F;	break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			optDoc |= MODRM_BYTE;	break;
		case 'a':
			optDoc |= ADD_REG;	break;
		case 'L':
			optDoc |= LOCK_OP;	break;
		case 'l':
			optDoc |= AFTER_LOCK;	break;
		case 'R':
			optDoc |= REP_INSTR;	break;
		case 'r':
			optDoc |= AFTER_REP;	break;
		case 'X':
			optDoc |= XTENDS;	break;
		case 't':
			optDoc |= TWO_OP_MULT;	break;
		case 'P':
			optDoc |= USE_REG;	break;
		default:
			error("Illegal flag %x in '%s'", *p, optf);
			return;
		}
	}
}

/* 
 * pick a random production. 
 * may retry if random production is wrong mode.
 */
static int pickRand(register oper *curoper)
{
	int i;
	char c, *p, state, work[22];

	if (bswitch && curoper->badct) {
		p = curoper->badlist;
		i = randl() % curoper->badct;
	}
	else {
		p = curoper->goodlist;
		i = randl() % curoper->goodct;
	}

	for (state = 0; (c = *p); p++) {
		if (state) {
			if (isspace(c))
				state = 0;
			continue;
		}
		if (isspace(c))
			continue;
		if (!i--)
			break;
		state = 1;
	}

	if (!c)
		error("Logic error in produce");

	/* put it out finding any internal productions */
	for (i = 0;;) {
		if (('%' == (c = *p++)) || isalnum(c)) {
			work[i++] = c;
			continue;
		}
		if (i) {	/* we have a word */
			work[i] = '\0';
			i = 0;
			if(!produce(work))
				return (0);
		}
		if (!c || isspace(c))
			break;
		if (otp) {
			fputc(c, otp);
			if (',' == c)
				fputc(' ', otp);
		}
	}
	return (1);
}

/*
 * Produce an operand. Returns 1 on success 0 for a wrong
 * production for a limited test. That is if we are testing
 * all small stuff curoper returns 0 is asked to produce %eax
 */
static int produce(char *n)
{
	register int j;
	register oper *curoper;

#ifdef TRACE
	if (otp) fprintf(otp, "{%s}", n);
#endif
	for (j = 0; j < operct; j++) {
		curoper = opertab + j;

		if (!strcmp(n, curoper->name)) {
			if (lswitch && !(curoper->flag & X_LARGE))
				return (0);	/* fail */
			if (sswitch && !(curoper->flag & X_SMALL))
				return (0);	/* fail */

			while (!pickRand(curoper))
				;
			return (1);
		}
	}

	if (sswitch && ('%' == n[0])) /* remove % from regs is small tst */
		n++;

	if (otp) fprintf(otp, "%s", n);
	return (1);
}

/*
 * make test file entrys.
 */
static void makeTst(char *n, int j)
{
	register funs *f;
	register opts *curoper;
	register int i;


	curoper = optab + j;
	f = funtab + curoper->fun;

	/* can we do curoper */
	if (lswitch)	/* large only test */
		for (i = 0; i < f->operands; i++)
			if (!(opertab[(unsigned char)f->ap[i]].flag & X_LARGE))
				return;

	if (sswitch)	/* small only test */
		for (i = 0; i < f->operands; i++)
			if (!(opertab[(unsigned char)f->ap[i]].flag & X_SMALL))
				return;

	if ((tmask && !(f->opt & tmask)) || (f->opt & nmask))
		return;

	if (otp) fprintf(otp, "\t%s\t", n);
	if (sswitch) {	/* reverse operands */
		for (i = f->operands; i--; ) {
			if (1 != (f->operands - i)) {
				if (otp) fprintf(otp, ", ");
			}
			produce(opertab[(unsigned char)f->ap[i]].name);
		}
	}
	else {		
		for (i = 0; i < f->operands; i++) {
			if (i) {
				if (otp) fprintf(otp, ", ");
			}
			produce(opertab[(unsigned char)f->ap[i]].name);
		}
	}
	if (otp) fprintf(otp, "\t/ %04x %04x\n", opt, opcode);
}

/*
 * Produce Document lines.
 */
void makeDoc(register funs *f)
{
	int i;

	if (!odp) return;
	if (-1 == curgen)
		fprintf(odp, "%s %d 2!", opc, lineno);
	else
		fprintf(odp, "%s %d 1!", curoperGen, lineno);

	if (optDoc & PFX_0F)
		fprintf(odp, "0F ");

	if (optDoc & FLOAT_PFX)
		fprintf(odp, "9B ");

	if ((opcode & 0xff00) || (optDoc & MODRM_BYTE))
		fprintf(odp, "%02x ", (opcode >> 8) & 255);

	if (optDoc & MODRM_BYTE)
		fprintf(odp, "/%o", opcode & 7);
	else
		fprintf(odp, "%02x", opcode & 255);

	if (optDoc & USE_REG)
		fprintf(odp, " /r");

	if (optDoc & ADD_REG)
		fprintf(odp, " +r");

	fprintf(odp, "\t\\fB%s\\fR", opc);

	if (f->operands) {
		fprintf(odp, "\t\\fI");
		for (i = 0; i < f->operands; i++) {
			char *n = opertab[(unsigned char)f->ap[i]].name;

			if (i)
				fprintf(odp, ",");

			fprintf(odp, "%s", strcmp(n, "atdx") ? n : "(dx)");
		}
		fprintf(odp, "\\fR");
	}
	if (*comment) {
		if (!f->operands)
			fprintf(odp, "\t");
		fprintf(odp, "\t%s", comment);
	}
	fputc('\n', odp);
}

/*
 * Build opcode and function entrys.
 */
void opBld(void)
{
	register opts *curoper;
	register funs *f = 0;  /* Pacify GCC about uninitialized variable. */
	int i, j, k;

	if (op3[0]) {
		ct = 3;
		sprintf(fname, "S_%04x_%s_%s_%s", opt, op1, op2, op3);
	}
	else if (op2[0]) {
		ct = 2;
		sprintf(fname, "S_%04x_%s_%s", opt, op1, op2);
	}
	else if (op1[0]) {
		ct = 1;
		sprintf(fname, "S_%04x_%s", opt, op1);
	}
	else {
		ct = 0;
		sprintf(fname, "S_%04x", opt);
	}

	for (j = 0; j < funct; j++)
		if (!strcmp(funtab[j].name, fname)) {
			f = funtab + j;
			break;
		}

	if (j == funct) {	/* not found build one */
		EXPAND(fun, funs);
		f = funtab + j;
		f->name = newcpy(fname);
		f->opt = opt;
		f->operands = ct;
		f->type = "OP";
		f->ap[0] = findOpr(op1);
		f->ap[1] = findOpr(op2);
		f->ap[2] = findOpr(op3);
	}

	i = opct;
	EXPAND(op, opts);
	curoper = optab + i;

	for (k = 0; k < i; k++) {
		if (!strcmp(optab[k].name, opc)) {
			curoper->name = optab[k].name;
			break;
		}
	}

	if (k == i)
		curoper->name = newcpy(opc);

	curoper->fun = j;
	curoper->opcode = opcode;
	curoper->gen = curgen;
	curoper->hash = -2;
	curoper->lineno = lineno;

	if (!(opt & AMBIG_MATCH)) {
		makeTst(opc, i);
		makeDoc(f);
	}
}

/*
 * Find operand on table or report error.
 */
int findOpr(char *name)
{
	int i;

	if (!name[0])
		return (-1);

	for (i = 0; i < operct; i++)
		if (!strcmp(opertab[i].name, name))
			return (i);

	error("undefined operand %s", name);
	return -1;
}

/*
 * Comparison routine by inverse name length, then name, then order given.
 */
int compr1(const void *vp1, const void *vp2)
{
	register int i;
	register const opts *p1 = (const opts*)vp1, *p2 = (const opts*)vp2;

	/* i32_t names then short */
	if ((i = strlen(p2->name) - strlen(p1->name)))
		return (i);

	/* alpha order */
	if ((i = strcmp(p1->name, p2->name)))
		return(i);

	return (p1->lineno - p2->lineno); /* in order given */
}

/*
 * Comparison routine by name length, then name, then input position.
 */
int compr2(const void *vp1, const void *vp2)
{
	register int i;
	register const opts *p1 = (const opts*)vp1, *p2 = (const opts*)vp2;

	/* short names then long */
	if ((i = strlen(p1->name) - strlen(p2->name)))
		return (i);

	/* alpha order */
	if ((i = strcmp(p1->name, p2->name)))
		return (i);

	return (p1->lineno - p2->lineno); /* in order given */
}

/*
 * Organize tables.
 */
void reorgData(void)
{
	register opts *curoper = 0;  /* Pacify GCC about uninitialized variable. */
	register opts *that, *last;
	char *p;
	int i, j, k;

	/* sort for creating allntab */
	qsort(optab, opct, sizeof(*optab), compr1);

	for (i = 0; i < opct; i++) { /* scan opcodes */
		curoper = optab + i;

		curoper->len = k = strlen(curoper->name);
		if (NULL == (p = strstr(allntab, curoper->name))) {
			/* if name not on list build */
			j = allnct;
			NEWN(alln, k);
			strcpy(allntab + j, curoper->name);
		}
		else
			j = p - allntab;

		curoper->pt = j;
	}

	/* sort for creating prefTab */
	qsort(optab, opct, sizeof(*optab), compr2);

	for (last = optab, nameCt = i = 0; i < opct; i++) { /* scan opcodes */
		curoper = optab + i;
		curoper->lineno = -1;
		curoper->count  = 0;
		if (-1 == curoper->fun) { /* general name */
			/* scan for reference */
			for (j = 0; j < opct; j++) {
				that = optab + j;
				if (-2 != that->hash ||
				    curoper->gen != that->gen)
					continue;
				that->gen = i;	/* general ref marked */
				that->hash = -1;
				curoper->count++;
				if (!strcmp(curoper->name, that->name))
					that->hash = -3; /* no unique name */
			}
		}

		/* 
		 * count names and mark first name in seq
		 * by leaving its pointer and count intact.
		 */
		if (strcmp(last->name, curoper->name)) {
			if (!last->count)
				last->count = curoper - last;
			last = curoper;
			nameCt++;
		}
		else
			curoper->pt = curoper->len = 0;
	}
	last->count = (curoper - last) + 1;
}

/*
 * Output all tables.
 */
void outData(void)
{
	register opts *curoper, *that;
	register funs *f;
	regs *r;
	char *p, work[22];
	int i, j, k, l;

	fprintf(ohp, "/*\n");
	fprintf(ohp, " * 80386 assembler header file.\n");
	fprintf(ohp, " * Generated by tabbld\n");
	fprintf(ohp, " */\n\n");

	fprintf(ofp, "/*\n");
	fprintf(ofp, " * 80386 assembler table file.\n");
	fprintf(ofp, " * Generated by tabbld\n");
	fprintf(ofp, " */\n");
	fprintf(ofp, "#include <stdio.h>\n\n");
	fprintf(ofp, "#include \"asm.h\"\n");
	fprintf(ofp, "#include \"intsize.h\"\n");
	fprintf(ofp, "#include \"y_tab.h\"\n");
	fprintf(ofp, "#include \"symtab.h\"\n\n");

	fprintf(ohp, "/* operand types */\n");
	/* dump base operand types */
	for (i = 0; i < operct; i++)
		if ((j = opertab[i].base))
			fprintf(ohp, "#define %-9s %2d\n", opertab[i].name, j);

	/* dump function table */
	fprintf(ohp, "\n/* instruction types */\n");
	fprintf(ofp, "const symt typTab[] = {\n");
	for (i = 0; i < funct;) {
		f = funtab + i;
		fprintf(ohp, "#define %-21s %2d\n", f->name, i);
		fprintf(ofp, " /* %-21s */ { %10s, 0x%04x, %d, {",
			f->name,
			f->type,
			f->opt & 0xffff,
			f->operands);
		if (f->operands) {
			for (j = 0; j < f->operands; j++)
				fprintf(ofp, "%s%s", &", "[j == 0], opertab[(unsigned char)f->ap[j]].name);
		} else {
			fputs(" 0", ofp);  /* C89 doesn't allow empty initializers. */
		}
		fprintf(ofp, " } }%s\n", ((++i < funct) ? "," : ""));
	}
	fprintf(ofp, "};\n\n");

	/* dump the name hash */
	fprintf(ofp, "const char charLump[] = {");
	i = 0;
	for (p = allntab; *p;) {
		switch (i++) {
		case 0:
			fprintf(ofp, "\n\t");
			break;
		case 10:
			i = 0;
			/* fallthrough */
		default:
			fprintf(ofp, " ");
		}
		fprintf(ofp, "'%c'", *p++);
		if (*p)
			fprintf(ofp, ",");
	}
	fprintf(ofp, "\n};\n\n");

	/*
	 * dump preftab first generic opcodes then regular.
	 * opcode is now used to point to the preftab address.
	 * for generic opcodes.
	 * lineno is now used tp point to the preftab address
	 * for processed opcodes that need hash pointers
	 * and zero for other processed opcodes.
	 */
	fprintf(ofp, "const opc prefTab[] = {\n");
	for (lastp = i = 0; i < opct; i++) {
		curoper = optab + i;
		if (curoper->fun != -1)	/* non generic opcode */
			continue;

		work[0] = '\0';
		curoper->opcode = lastp;

		fputc('\n', ofp);
		for (j = 0; j < opct; j++) {
			that = optab + j;
			if (that->gen != i || that->fun == -1)
				continue;
			that->lineno = 0;
			/* remember first of each name */
			if(strcmp(work, that->name)) {
				k = j;
				optab[k].lineno = l = lastp;
				strcpy(work, that->name);
			}
			fprintf(ofp,
				"\t{ (short)0x%04x, %21s },\t/* %-10s %d */\n",
				that->opcode & 0xffff,
				funtab[that->fun].name,
				that->name,
				lastp);
			lastp++;
		}
	}
	fputc('\n', ofp);

	/* do non generic opcodes */
	for (i = 0; i < opct; i++) {
		curoper = optab + i;
		/* regular first opcode */
		if (curoper->fun != -1 && curoper->len) {
			if (-1 != curoper->lineno) { /* matches end of generic */
				curoper->opcode = curoper->lineno;
				continue;
			}
			k = lastp;	/* save lastp */
			for (j = i; j < opct;) {
				that = optab + j;
				if (strcmp(curoper->name, that->name))
					break;
				if (-1 == that->fun) {
					errors++;
					fprintf(stderr, "odd order %s",
						that->name);
					break;
				}
				fprintf(ofp,
				   "\t{ (short)0x%04x, %21s }%s\t/* %-10s %d */\n",
					that->opcode & 0xffff,
					funtab[that->fun].name,
					((++j != opct) ? "," : ""),
					that->name,
					lastp);
				lastp++;
			}
			curoper->opcode = k;
		}
	}
	fprintf(ofp, "};\n\n");

	free(funtab);

	/* set up hash table to mark */
	htab = (short*)alloc(nameCt * sizeof(*htab));
	fprintf(ohp, "#define OPCOUNT %d /* count of opcodes */\n", nameCt);

	for (i = 0; i + 0U < nameCt; i++)
		htab[i] = -1;

	/* mark all items that hash direct */
	for (i = 0; i < opct; i++) {
		curoper = optab + i;

		curoper->hash = -2;
		if (!curoper->len)
			continue;
		memcpy(work, allntab + curoper->pt, curoper->len);
		work[curoper->len] = '\0';
		j = hash(work) % nameCt;

		if (htab[j] != -1) {	/* hole taken get it next pass */
			curoper->hash = -1;
			continue;
		}
		htab[j] = i;	/* hash table points to entry */
	}

	/* mark items that hash indirect */
	for (i = 0; i < opct; i++) {
		curoper = optab + i;
		if (-1 != curoper->hash)	/* pass unmarked items */
			continue;

		 /* find a hole */
		for (k = 0;(k + 0U < nameCt) && (htab[k] != -1); k++)
			;
		if (k + 0U == nameCt) {
			errors++;
			fprintf(stderr, "Insufficient holes in table");
			continue;
		}
		htab[k] = i;	/* hash table points to entry */

		/* where does curoper hash to */
		memcpy(work, allntab + curoper->pt, curoper->len);
		work[curoper->len] = '\0';
		j = hash(work) % nameCt;

		/* find the end of the chain */
		while(-2 != (j = (that = optab + htab[j])->hash))
			;

		that->hash = k;
		curoper->hash = -2;
	}

	fprintf(ofp, "nhash hashCodes[] = {\n");
	for (i = 0; i + 0U < nameCt;) {
		j = htab[i++];
		if (j < 0 || j > opct) {
			errors++;
			fprintf(stderr, "Unplaned hole in table");
			fprintf(ofp, "\t{-1, 0, 0, 0, 0 }, /* JUNK */\n");
			continue;
		}
		curoper = optab + j;

		memcpy(work, allntab + curoper->pt, curoper->len);
		work[curoper->len] = '\0';
		fprintf(ofp, "\t{%4d, %4d, %2d, %2d, %4d }%c /* %-12s %d */\n",
			((curoper->hash < 0) ? -1 : curoper->hash),
			curoper->pt,
			curoper->len,
			curoper->count,	/* entries on pref table */
			curoper->opcode,	/* entry on pref table */
			((i + 0U == nameCt) ? ' ' : ','),
			work,
			i - 1);
	}
	fprintf(ofp, "};\n\n");

	fprintf(ohp,
		"#define SYMCOUNT %d\t/* count of predefined symbols */\n",
		regct + 1);

	fprintf(ofp, "psym symtab[] = {\n");
	fprintf(ofp,	/* too fancy for the basic mechinism */
		"\t{NULL, IDENTIFIER, 0, 0, 1, 0, (sym*)symtab, 0, 0, \".\" },\n");
	for (i = 0; i < regct; ) {
		r = regtab + i++;

		j = 0;
		if (NULL == strstr(r->ytype, "REG"))
			strcpy(work, r->name);
		else {
			sprintf(work, "%%%s", r->name);
			switch(r->ytype[0]) {
			case 'S':	/* SEG_REG */
				j = 0x800;
				break;
			case 'C':	/* CTL_REG */
				j = 0x400;
				break;
			case 'D':	/* DEB_REG */
				j = 0x200;
				break;
			case 'T':	/* TST_REG */
				j = 0x100;
				break;
			}
			r->ytype = "REG";
		}
		fprintf(ofp,
		  "\t{ NULL, %10s, %d, %d, 0, %d, NULL, 0, 0, \"%s\" }%s\n",
		  r->ytype,
		  r->loc,
		  r->len,
		  j,
		  work,
		  ((i < regct) ? "," : ""));
	}
	fprintf(ofp, "};\n");
}

/*
 * Process opcode files.
 */
int main(int argc, char *argv[])
{
	int c, subtest;

	for (subtest = 0; EOF != (c = getargs(argc, argv, "bdelst:n:?"));) {
		switch (c) {
		case 'b':
			bswitch = 1;
			break;
		case 'l':
			lswitch = 1;
			break;
		case 's':
			sswitch = 1;
			break;
		case 'd':
			dswitch = 1;
			continue;
		case 'e':
			eswitch = 1;
			continue;
		case 't':
			tabbld_sscanf(as_optarg, "%x", &tmask);
			break;
		case 'n':
			tabbld_sscanf(as_optarg, "%x", &nmask);
			break;
		case '?':
		default:
			fprintf(stderr,
	 "usage: tabbld [-bls] [-t bits_to_match] [-n bits_not_to_match]\n");
	 		exit (1);
		}
		subtest = 1;	/* any options are a subtest */
	}
			
	/*
	 * These are nessisary as long as curoper runs small model.
	 * otherwise curoper thing runs out of space.
	 */
	START(op,  1000, opts);
	START(fun, 250, funs);
	START(oper, 100, oper);
	START(alln, 2000, char);
	START(reg, 100, regs);

	otp = eswitch ? NULL : xopen("test.s", "w");
	if (subtest) {
		if (otp) {
		fprintf(otp, "\t.ttl\tSubtest of asm 386 ");
		if (bswitch | lswitch | sswitch)
			fputc('-', otp);
		if (bswitch)
			fputc('b', otp);
		if (lswitch)
			fputc('t', otp);
		if (sswitch)
			fputc('s', otp);
		if (tmask)
			fprintf(otp, " -t %x", tmask);
		if (nmask)
			fprintf(otp, " -n %x", nmask);
		fputc('\n', otp);
		}
		odp = ohp = ofp = xopen("/dev/null", "w");
	}
	else {
		if (otp) fprintf(otp, "\t.ttl\tFull test of asm 386\n");
		ofp = xopen("symtab.c", "w");	/* symbol table */
		ohp = xopen("symtab.h", "w");	/* header file */
		odp = dswitch ? NULL : xopen("document", "w");	/* document file */
	}
	if (otp) fprintf(otp, "\t.llen\t100\n");
	if (otp) fprintf(otp, "abc:\n");
	
	/*
	 * Process file.
	 */
	while (NULL != (line = as_getline(stdin, &lineno))) {
		switch (*line) {
		case '+':
			tabbld_sscanf(line + 2, "%d", &state);
			continue;
		case 0:
			if (1 == state) {	/* pass through comments */
				if (!odp) {}
				else if (-1 == curgen)
					fprintf(odp, "%s %d 2!\t%s\n",
						((optDoc & INDEF_JMP) ? "ja" : opc),
						lineno, comment);
				else
					fprintf(odp, "%s %d 1!\t%s\n",
						curoperGen, lineno, comment);
			}
			continue;
		}
		switch(state) {
		case 0:	/* test stream directives. */
			buildTst();
			break;
		case 1: /* opcodes */
			buildOp();
			break;
		case 2: /* registers */
			buildReg();
			break;
		case 3: /* assembler directives */
			buildDir();
		}
	}
	if (otp) fclose(otp);

	if (subtest)
		return (0);

	reorgData();
	outData();
	if (errors)
		fprintf(stderr, "%d error%c detected\n", errors, 
			(1 == errors) ? ' ' : 's');
	showStats(errors ? 1 : 0);

	return (0);
}
