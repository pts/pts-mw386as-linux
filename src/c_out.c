/*
 * coff format output handler.
 */
#include <asm.h>
#include <coff.h>
#include <utype.h>
#include <symtab.h>

#define CMOD 2		/* clump every 2 bytes */
#define LIMIT 10	/* bytes of data per line */
#define STARTP 31	/* start source at. Must be n * 8 -1 */
#define DATABUF 512	/* size of data buffer */
#define RELBUF 16	/* entries in relocation buffer */

#define ROUNDR 3	/* used to round segments */

typedef struct seg seg;
struct seg {		/* coff section header */
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address, aliased s_nlib */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */

	/* Extra data connected with this to allow running this segment */
	long		data_seekad;	/* next data seek address */
	long		relo_seekad;	/* next relocation seek address */
	long		curadd;		/* current address this segment */
	long		hiadd;		/* hi address this pass */
	short		segNo;		/* file output number for this seg */
	short		segSeq;		/* Segment sequence no */

	char *bp;			/* data buffer pointer */
	RELOC *relBp;			/* relocation buffer pointer */
	char buf[DATABUF];		/* data buffer */
	RELOC relBuf[RELBUF];		/* relocation entry buffer */
};

static long *tracker;	/* for debugging */

static seg *segs, *segend;	/* segment bases */
static seg *cseg;		/* current segment */

static unsigned short ct, pos, sects, symbs;
static long datapos, relpos, sympos, strOff;
static long lastSeek;
static FILE *ofp;		/* output file */

union word {
	unsigned char uc[2];
	unsigned short us;
};

union full {
	unsigned char uc[4];
	unsigned long ul;
};

struct symdef {
	symdef	*next;
	union {
		SYMENT s;
		AUXENT a;
	} u;
};
static symdef *this;
static long symct;

#define SYM this->u.s
#define AUX this->u.a

/*
 * output spaces and tabs to a position, then output a character.
 */
static void
outTo(to, s)
{
	int i;

	if(!pos)
		sTitle();

	for (to += (' ' == s); (i = ((pos | 7) + 1)) <= to; pos = i)
		putchar('\t');

	for (; pos < to; pos++)
		putchar(' ');

	if (' ' != s)
		putchar(s);
}

/*
 * Do titles.
 */
sTitle()
{
	static short pageno;

	if(pswitch && (++linect > nlpp)) {
		if(pageno)
			printf("\n\n\n\n\n");
		printf(
	"\n\nMark Williams 80386 Assembler\t%s\t%s\tPage %d\n\n",
			dTime, title, ++pageno);
		linect = 0;
	}
}

/*
 * output source line
 */
outLine(p, s)
char *p, s;
{
	register char c;
		
	if ((2 != pass) ||
	    !lswitch ||
	    (!mlistsw && (NULL != macExp)) ||
	    (NULL == p))
		return;

	outTo(STARTP, s);
	for (; (c = *p++) && ('\n' != c); pos++) {
		if (lineSize <= pos) {
			putchar('\n');
			pos = 0;
			outTo(STARTP, '|');
		}
		if ('\t' == c)
			pos |= 7;
		putchar(c);
	}
	putchar('\n');
	pos = ct = 0;
}

/*
 * Write to obj file.
 */
static void
owrite(buf, size)
char *buf;
{
	if (1 != fwrite(buf, size, 1, ofp))
		fatal("Write error on object file");	/**/
	lastSeek += size;
}

/*
 * Seek then Write to obj file.
 */
xwrite(add, buf, size)
long add;
char *buf;
{
	if (add != lastSeek) {
		if (fseek(ofp, add, 0))
			fatal("Seek error on object file");	/**/
		lastSeek = add;
	}
	owrite(buf, size);
}

/*
 * Write actual data.
 */
outData(s)
register seg *s;
{
	register size;

	if (size = s->bp - s->buf) {
		if (s->s_flags != STYP_BSS) {
			xwrite(s->data_seekad, s->buf, size);
			s->data_seekad += size;
		}
		s->bp = s->buf;
	}
}

/*
 * Write relocation records.
 */
outRel(s)
register seg *s;
{
	register size;

	if (size = (char *)s->relBp - (char *)s->relBuf) {
		xwrite(s->relo_seekad, (char *)s->relBuf, size);
		s->relo_seekad += size;
	}
	s->relBp = s->relBuf;
}

/*
 * put object data byte to buffer.
 */
binout(b, s)
unsigned short b;
register seg *s;
{
	if (s->bp == (s->buf + DATABUF))
		outData(s);
	*s->bp++ = b;
}


/*
 * Limit data on line. This allows cuts at logical points.
 */
static void
limiter(n)
{
	if (lswitch && (ct + n) >= LIMIT) {
		putchar('\n');
		ct = 0;
	}
}

/*
 * list data
 */
outlst(b)
unsigned short b;
{
	if (dot.sg == 3) {	/* BSS */
		static int statem = -1;	/* only one err per line */

		if (statement != statem) {
			statem = statement;

			yyerror("Data defined in .bss");
			/* The \fB.bss\fR segment is uninitialized data.
			 * You cannot place actual values there. */
		}
	}
	binout(b, cseg);

	if (!lswitch)
		return;

	limiter(0);

	if (!ct) {
		sTitle();
		pos = 4;
		printf("%04lx", dot.loc + segs[dot.sg - 1].s_vaddr);
	}

	if (!(ct % CMOD)) {
		pos++;
		putchar(' ');
	}
	printf("%02x", b & 255);
	pos += 2;
	ct++;
}

/*
 * output unrelocated byte
 */
outab(b)
unsigned short b;
{
	if (2 == pass)
		outlst(b);
	dot.loc++;	/* update location ctr */
}

/*
 * output unrelocated word.
 */
outaw(u)
unsigned short u;
{
	if (2 == pass) {
		union word w;

		w.us = u;
		limiter(2);
		outlst(w.uc[0]);
		outlst(w.uc[1]);
	}
	dot.loc += 2;
}

/*
 * output unrelocated long.
 */
outal(ul)
long ul;
{
	if (2 == pass) {
		union full l;

		l.ul = ul;
		limiter(4);
		outlst(l.uc[0]);
		outlst(l.uc[1]);
		outlst(l.uc[2]);
		outlst(l.uc[3]);
	}
	dot.loc += 4;
}

/*
 * output relocation.
 */
static long
relOut(oper, sw)
register expr *oper;
unsigned sw;
{
	register RELOC *bp;
	register sym *sp;
	register long rv;

	rv = oper->exp;
	if (NULL == (sp = oper->ref) || (sp->type & S_XSYM))
		return(rv);	/* absolute addr */

	bp = cseg->relBp++;
	bp->r_type   = sw;
	bp->r_vaddr  = dot.loc + cseg->s_vaddr;

#if 0
	/*
	 * If this symbol is not for the symbol table
	 * relocation record uses the segment symbols
	 */
	if (!sp->num)
#else
	/*
	 * Only use symbol references when forced
	 * seems to be required by a bug in the
	 * ESIX linker.
	 */
	if (!(sp->flag & (S_EXREF|S_COMMON)))
#endif
		bp->r_symndx = sp->sg - 1;
	else {
		bp->r_symndx = sp->num;
		if (sp->flag & S_COMMON)
			rv += sp->size;
		rv -= sp->loc;
	}

	if (bp == (cseg->relBuf + (RELBUF - 1)))
		outRel(cseg);

	/* gimmic addr to make .data after .text */
	if ((sp->sg > 1) && !(sp->flag & S_COMMON))
		rv += segs[sp->sg - 1].s_vaddr;

	return(rv);
}
		
/*
 * output relocated byte
 */
outrb(oper, sw)
expr *oper;
int sw;		/* 0 = Relative address, 1 = PC relative address */
{
	if (2 == pass) {
		if (sw)
			outlst((unsigned short)(relOut(oper, R_PCRBYTE) -
						(dot.loc + 1)));
		else
			outlst((unsigned short)relOut(oper, R_RELBYTE));
	}
	else if (NULL != oper->ref)
		cseg->s_nreloc++;
	dot.loc++;
}

/*
 * output relocated word.
 */
outrw(oper, sw)
expr *oper;
int sw;		/* 0 = Relative address, 1 = PC relative address */
{
	union word w;

	if (2 == pass) {
		if (sw)
			w.us = relOut(oper, R_PCRWORD) - (dot.loc + 2);
		else
			w.us = relOut(oper, R_DIR16);

		limiter(2);
		outlst(w.uc[0]);
		outlst(w.uc[1]);
	}
	else if (NULL != oper->ref)
		cseg->s_nreloc++;
	dot.loc += 2;
}

/*
 * output relocated long.
 */
outrl(oper, sw)
expr *oper;
int sw;		/* 0 = Relative address, 1 = PC relative address */
{
	union full l;

	if (2 == pass) {
		if (sw)
			l.ul = relOut(oper, R_PCRLONG) - (dot.loc + 4);
		else
			l.ul = relOut(oper, R_DIR32);

		limiter(4);
		outlst(l.uc[0]);
		outlst(l.uc[1]);
		outlst(l.uc[2]);
		outlst(l.uc[3]);
	}
	else if (NULL != oper->ref)
		cseg->s_nreloc++;
	dot.loc += 4;
}

/*
 * Output symbol table string.
 */
outSymStr (sp)
register sym *sp;
{
	register i;
	register char *name;

	name = SYMNAME(sp);
	if ((i = strlen(name)) > SYMNMLEN)
		owrite(name, i + 1);
}

/*
 * output symbol table entry.
 */
SYMENT *
outSym(sp, writesw)
register sym *sp;
{
	static SYMENT s;
	int i;
	register char *name;

	clear(&s);

	name = SYMNAME(sp);
	if ((i = strlen(name)) > SYMNMLEN) {
		s._n._n_n._n_offset = strOff;
		strOff += i + 1;
	}
	else
		strcpy(s._n._n_name, name);

	
	s.n_scnum = sp->sg;
	s.n_value = sp->loc;
	s.n_sclass = C_STAT;

	if (sp->flag & S_EXDEF) {
		s.n_sclass = C_EXT;
		if (sp->flag & S_COMMON) { /* common */
			s.n_value = (sp->size ? sp->size : 1);
			s.n_scnum = 0;
		}
	}
	if (sp->flag & S_EXREF) {
		s.n_scnum  = 0;
		s.n_value  = 0;
		s.n_sclass = C_EXT;
	}

	/* gimmic addr to make .data after .text */
	if (s.n_scnum > 1)
		s.n_value += segs[sp->sg - 1].s_vaddr;

	if (writesw)
		owrite((char *)&s, sizeof(s));
	return (&s);
}

/*
 * Go from pass 0 to pass 1 or 2
 */
newPass(fn)
char *fn;
{
	register seg *s;
	unsigned size;
	long xaddr;
	
	if ((cseg->curadd = dot.loc) > cseg->hiadd)
		cseg->hiadd = cseg->curadd;
	cseg = segs;	/* return to .text */
	pass = dot.sg = 1;
	defCt = macNo = dot.loc = 0;

	if (indPass()) {	/* take an extra pass */
		for (s = segs; s < segend; s++) {
			s->bp = s->buf;
			s->relBp = s->relBuf;
			s->s_nreloc = s->hiadd = s->curadd = 0;
		}
		symGlob(3); /* fix symbol table */
		return;
	}

	pass   = 2;	/* last pass */
	linect = nlpp;
	xaddr = datapos = relpos = 0;
	ofp = xopen(fn, "wb");
	
	for (s = segs; s < segend; s++) {
		s->s_paddr = s->s_vaddr = xaddr;
		s->hiadd += ROUNDR;		/* Round up */
		s->hiadd &= ~ROUNDR;
		xaddr += s->s_size = s->hiadd;
		s->data_seekad = datapos; /* temp value */
		s->s_relptr = relpos;
		if (s->s_flags != STYP_BSS) {
			datapos += s->hiadd;
			relpos  += s->s_nreloc * RELSZ;
		}
	}
	size =  sizeof(FILEHDR) + (sects * sizeof(SCNHDR));
	sympos = size + datapos + relpos;
	relpos = size + datapos;

	if (fseek(ofp, lastSeek = sympos, 0))
		fatal("Seek error on object file");	/**/

	for (s = segs; s < segend; s++) {
		SYMENT sym;

		/* write symbol records for segments */
		clear(&sym);
		sym.n_sclass = C_STAT;
		sym.n_value = s->s_vaddr;
		strcpy(sym._n._n_name, s->s_name);
		sym.n_scnum = s->segSeq;
		owrite((char *)&sym, sizeof(sym));

		if (s->hiadd && (s->s_flags != STYP_BSS))
			s->s_scnptr = s->data_seekad += size;
		else
			s->s_scnptr = s->data_seekad = 0;

		if (s->s_nreloc && (s->s_flags != STYP_BSS))
			s->relo_seekad = s->s_relptr += relpos;
		else
			s->relo_seekad = s->s_relptr = 0;

		s->hiadd = s->curadd = 0;
	}

	strOff = 4;			/* init long symbol length */
	symbs = symGlob(3);		/* write symbol table */

	if (strOff > 4) {
		owrite(&strOff, sizeof(strOff)); /* write length of tail */
		symDump();		   /* dump symbols that are too long */
	}
}

/*
 * write record header then segment headers.
 */
static void
writeHeader()
{
	register seg *s;
	FILEHDR head;
	long loadd;

	head.f_magic = C_386_MAGIC;
	time(&head.f_timdat);
	head.f_nsyms = symbs;
	head.f_symptr = sympos;
	head.f_opthdr = 0;
	head.f_flags = F_LNNO;
	head.f_nscns = sects;
	
	xwrite(0L, (char *)&head, sizeof(head));
	for (loadd = 0, s = segs; s < segend; s++)
		owrite(s, sizeof(SCNHDR));
}

/*
 * Finish any output.
 */
cleanUp()
{
	int pad;
	register seg *s;

	s = cseg;		/* close out current seg */
	if(s->hiadd < (s->curadd = dot.loc))
		s->hiadd = s->curadd;

	writeHeader();
	for (s = segs; s < segend; s++) {
		if (s->s_flags == STYP_BSS)
			continue;

		/* go to end of segment */
		if (s->curadd != s->hiadd)
			doOrg(NULL, NULL);

		/* pad with nop or zero */
		pad = (s->s_flags == STYP_TEXT) ? NOP : 0;
		for (; s->hiadd & ROUNDR; s->hiadd++)
			binout(pad, s);
		outData(s);
		outRel(s);
	}
}

/*
 * org command
 */
doOrg(label, oper)
parm *label;
data *oper;
{
	register seg *s;
	long from, to;
	char pad;

	buildlab(label);

	from = dot.loc;
	s = cseg;
	if (pass == 2)
		outData(s);
	s->curadd = dot.loc;		/* first make segment current */
	if (dot.loc > s->hiadd)
		s->hiadd = dot.loc;
	
	/* process org */
	switch ((NULL == oper) ? 'n' : oper->type) {
	case 'n':
		to = s->hiadd;	/* org to hi spot in segment */
		break;
	case 'd':
	case 's':
		yyerror("Org to invalid value");
		/* You may not \fB.org\fR to doubles or strings. */
		return (1);
	case 'y':
		if (oper->d.y->sg != dot.sg) {
			yyerror("Org to wrong segment");
			/* You must \fB.org\fR to the current segment. */
			return (1);
		}
		to = oper->d.y->loc;
		break;
	case 'l':
		to = oper->d.l;	/* org where requested */
	}

	if ((pass != 2) || (s->s_flags == STYP_BSS)) { /* no disk work */
		s->curadd = dot.loc = to;	/* make segment current again */
		if (to > s->hiadd)
			s->hiadd = dot.loc;
		return (0);
	}
	if (to <= s->hiadd) {	/* no pad if we've been there */
		s->curadd = dot.loc = to;
		s->data_seekad += to - from;
		return (0);
	}
	from = s->curadd = dot.loc = s->hiadd;	/* seek to the old end */
	s->data_seekad += s->hiadd - from;

	pad = (s->s_flags == STYP_TEXT) ? NOP : 0; /* pad out */
	for (;from < to; from++)
		binout(pad, s);
	s->curadd = dot.loc = to;	/* make segment current again */
	if (to > s->hiadd)
		s->hiadd = dot.loc;
	return (0);
}

/*
 * Initialize segment information.
 */
segInit()
{
	int i;
	register seg *s;

	static char *segclass[] = {
		".text", ".data", ".bss"
	};
	static long segflag[] = {
		STYP_TEXT, STYP_DATA, STYP_BSS
	};

	cseg = segs = alloc(sizeof(*segs) * 3);
	segend = segs + 3;

	for (i = 0; i < 3; i++) {
		s = segs + i;
		strcpy(s->s_name, segclass[i]);
		s->s_flags = segflag[i];
		s->segSeq = ++sects;
		s->bp = s->buf;
		s->relBp = s->relBuf;		
	}
}

/*
 * .ident and .version set stuff in the comment section.
 */
cmnt(op, p)
opc *op;
parm *p;
{
}

/*
 * coff statements after def.
 */
coffdef(s)
sym *s;
{
	if (NULL != this)
		yywarn("Missing .endef");
		/* The previous .def was not closed by a .endef */
	this = alloc(sizeof(*this));
	SYM.n_sclass = C_STAT;	/* defaults in case */
	SYM.n_value  = s->loc;
	
}

static void
checkthis()
{
	if (NULL == this)
		yyerror("A .def command must be in effect");
		/* This command is only legal between a .def and a .endef */
}

coffendef()
{
	checkthis();
	this = NULL;
}

coffval(item)
data *item;
{
	SYMENT *s;

	checkthis();
	if ('y' == item->type) {
		s = outSym(item->d.y, 0);
		SYM.n_value = s->n_value;
		SYM.n_scnum = s->n_scnum;
	}
	else {
		SYM.n_value = item->d.l;
		SYM.n_scnum = 0;
	}
		
}

coffset(op, n)
opc *op;
long n;
{
	checkthis();
	switch(op->kind) {
	case S_SCL:
		SYM.n_sclass = n;
		break;
	case S_TYPE:
		SYM.n_type   = n;
		break;
	}
}

coffln(op, p, n)
opc *op;
parm *p;
long n;
{
}

/*
 * .comm and .lcomm commands.
 * Improve later.
 */
comm(op, p, n)
opc *op;
parm *p;
long n;
{
	sym *s;

	if (NULL == p) {
		yyerror(".comm must have tag");
		/* The format of \fB.comm\fR is \fB.comm name, size\fR. */
		return;
	}
	if (op->code == 2) {
		s = symLookUp(p->str, S_LOCAL, 0L, op->code + 1);
		s->size = n;
		s->flag = S_EXDEF|S_COMMON;
	}
}

/*
 * segment command.
 */
sym *
segment(op, p, n)
opc *op;
parm *p;
long n;
{
	register seg *s;
	sym *rv;

	s = segs + op->code;

	if (NULL == p) {	/* segment change */
		if ((cseg->curadd = dot.loc) > cseg->hiadd)
			cseg->hiadd = cseg->curadd;

		cseg = s;
		dot.loc = s->curadd;
		dot.sg = op->code + 1;
		return (NULL);
	}

	/* set up data */
	rv = symLookUp(p->str, S_LOCAL, s->curadd, op->code + 1);
	rv->size = n;
	s->curadd += n;
	if (s->curadd > s->hiadd)
		s->hiadd = s->curadd;
	return (rv);
}
