/*
 * coff format output handler.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "asm.h"
#include "intsize.h"
#include "coff.h"
#include "utype.h"
#include "symtab.h"

void writeDebug();
void writeDebugLong();
void doOrg(parm *label, data *oper);

#define WHERE 0x40A09C

#define trace(n) printf("Trace %d\n", n);

#define CMOD 2		/* clump every 2 bytes */
#define LIMIT 10	/* bytes of data per line */
#define STARTP 31	/* start source at. Must be n * 8 -1 */
#define DATABUF 512	/* size of data buffer */
#define RELBUF 128	/* entries in relocation buffer */
#define LINEBUF 128	/* entries in line number buffer */
#define MANSEG 3	/* Segments that must go out */
#define ROUNDR 3	/* used to round segments */

typedef struct seg seg;
struct seg {		/* coff section header */
	char		s_name[8];	/* section name */
	i32_t		s_paddr;	/* physical address, aliased s_nlib */
	i32_t		s_vaddr;	/* virtual address */
	i32_t		s_size;		/* section size */
	i32_t		s_scnptr;	/* file ptr to raw data for section */
	i32_t		s_relptr;	/* file ptr to relocation */
	i32_t		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	i32_t		s_flags;	/* flags */

	/* Extra data connected with this to allow running this segment */
	i32_t		data_seekad;	/* next data seek address */
	i32_t		relo_seekad;	/* next relocation seek address */
	i32_t		line_seekad;	/* next line seek address */
	i32_t		curadd;		/* current address this segment */
	i32_t		hiadd;		/* hi address this pass */
	short		segSeq;		/* Segment sequence no */

	char *bp;			/* data buffer pointer */
	RELOC *relBp;			/* relocation buffer pointer */
	LINENO *lineBp;			/* line number buffer pointer */
	char buf[DATABUF];		/* data buffer */
	RELOC relBuf[RELBUF];		/* relocation entry buffer */
	LINENO lineBuf[LINEBUF];	/* line number entry buffer */
};

static seg *segs, *segend;	/* segment bases */
static seg *cseg;		/* current segment */

static unsigned short ct, pos, sects, symbs, usects;
static i32_t datapos, relpos, linepos, sympos, debpos;
static i32_t strOff = 4;
static i32_t lastSeek;
static FILE *ofp;		/* output file */

static int coffDefCt;		/* count of .def in file */
static int coffAuxCt;		/* aux records to be written */
static int coffEfcnCt;		/* end of function records */
static int txtAt;		/* DEBUG_RECS from last pass */
#define DEBUG_RECS (coffDefCt + coffAuxCt - coffEfcnCt)
#define USECTS (txtAt ? usects * 2 : usects)
static char defSw;		/* in a .def statement */
static char auxSw;
static char efcnSw;

struct xsym {	/* Symbol for debugging becomes SYMENT and maybe auxent */
	int	symno;
	int	value;
	short	scnum;
	short	type;
	signed char	sclass;
	char	numaux;
	AUXENT	aux;	/* aux record data */
	char	*name;	/* name if there is one */
};

static struct xsym *syms;		/* all the debug stuff */
static struct xsym *symptr;		/* currently build debug stuff */

union word {
	unsigned char uc[2];
	unsigned short us;
};

union full {
	unsigned char uc[4];
	u32_t ul;
};

/*
 * output spaces and tabs to a position, then output a character.
 */
static void
outTo(int to, int s)
{
	int i;

	if(!pos)
		sTitle();

	for (to += (' ' == s); (i = ((pos | 7) + 1)) <= to; pos = i)
		fputc('\t', stdout);

	for (; pos < to; pos++)
		fputc(' ', stdout);

	if (' ' != s)
		fputc(s, stdout);
}

/*
 * Do titles.
 */
void sTitle(void)
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
void outLine(char *p, char s)
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
			fputc('\n', stdout);
			pos = 0;
			outTo(STARTP, '|');
		}
		if ('\t' == c)
			pos |= 7;
		fputc(c, stdout);
	}
	fputc('\n', stdout);
	pos = ct = 0;
}

/*
 * Write to obj file.
 */
static void
owrite(const char *buf, int size)
{
	if (1 != fwrite(buf, size, 1, ofp))
		fatal("Write error on object file");	/**/
	lastSeek += size;
}

/*
 * Seek then Write to obj file.
 */
void xwrite(i32_t add, char *buf, int size)
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
void outData(s)
register seg *s;
{
	register int size;

	if ((size = s->bp - s->buf)) {
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
void outRel(s)
register seg *s;
{
	register int size;

	if ((size = (char *)s->relBp - (char *)s->relBuf)) {
		xwrite(s->relo_seekad, (char *)s->relBuf, size);
		s->relo_seekad += size;
	}
	s->relBp = s->relBuf;
}

/*
 * Write Line records.
 */
void outLineRec(s)
register seg *s;
{
	register int size;

	if ((size = (char *)s->lineBp - (char *)s->lineBuf)) {
		xwrite(s->line_seekad, (char *)s->lineBuf, size);
		s->line_seekad += size;
	}
	s->lineBp = s->lineBuf;
}

/*
 * put object data byte to buffer.
 */
void binout(b, s)
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
limiter(int n)
{
	if (lswitch && (ct + n) > LIMIT) {
		fputc('\n', stdout);
		ct = 0;
	}
}

/*
 * list data
 */
void outlst(b)
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

	if (!ct) {
		sTitle();
		pos = 4;
		printf("%04"PRIX32"", dot.loc + segs[dot.sg - 1].s_vaddr);
	}

	if (!(ct % CMOD)) {
		pos++;
		fputc(' ', stdout);
	}
	printf("%02X", b & 255);
	pos += 2;
	ct++;
}

/*
 * output unrelocated byte
 */
void outab(unsigned short b)
{
	if (2 == pass) {
		limiter(1);
		outlst(b);
	}
	dot.loc++;	/* update location ctr */
}

/*
 * output unrelocated word.
 */
void outaw(unsigned short u)
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
 * output unrelocated i32_t.
 */
void outal(i32_t ul)
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
static i32_t
relOut(oper, sw)
register expr *oper;
unsigned sw;
{
	register RELOC *bp;
	register sym *sp;
	register i32_t rv;

	rv = oper->exp;
	if (NULL == (sp = oper->ref) || (sp->type & S_XSYM))
		return(rv);	/* absolute addr */

	bp = cseg->relBp++;
	bp->r_type   = sw;
	bp->r_vaddr  = dot.loc + cseg->s_vaddr;

	/*
	 * Only use symbol references when forced
	 * seems to be required by a bug in the
	 * ESIX linker.
	 */
	if (!(sp->flag & (S_EXREF|S_COMMON))) {
		int seg = sp->sg == 0 ? 0 : segs[sp->sg - 1].segSeq - 1;  /* 0 only when failing for undefined symbols */

		bp->r_symndx = txtAt ? (seg * 2) + txtAt : seg;
	}
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
void outrb(expr *oper, int sw)
/* sw: 0 = Relative address, 1 = PC relative address */
{
	if (2 == pass) {
		limiter(1);
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
void outrw(expr *oper, int sw)
/* sw: 0 = Relative address, 1 = PC relative address */
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
 * output relocated i32_t.
 */
void outrl(expr *oper, int sw)
/* sw: 0 = Relative address, 1 = PC relative address */
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
void outSymStr(sp)
register sym *sp;
{
	register int i;
	register char *name;

	name = SYMNAME(sp);
	if ((i = strlen(name)) > SYMNMLEN)
		owrite(name, i + 1);
}

#if 0
static void showIt(sp)
sym *sp;
{
	int i;
	char *name;

	i = strlen(name = SYMNAME(sp));
	fprintf(stderr, "Trace %X, '%s' flag %d len %d num %d\n",
		 sp, name, sp->flag, i, sp->num);
}
#endif

/*
 * output symbol table entry.
 */
void outSym(sp)
register sym *sp;
{
	static SYMENT s;
	int i;
	register char *name;

	clear(&s);

	i = strlen(name = SYMNAME(sp));

	if (i > SYMNMLEN) {
		s.n_zeroes = 0;
		s.n_offset = strOff;
		strOff += i + 1;
	}
	else
		strncpy(s._n._n_name, name, SYMNMLEN);

	if (sp->sg <= 0 || !segs)  /* It is not 0 on normal COFF generation, but when an error is reported, it can be 0, thus causing the other end of the branch to segfault. */
		s.n_scnum = sp->sg;
	else
		s.n_scnum = segs[sp->sg - 1].segSeq;
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

	owrite((const char*)&s, sizeof(s));
}

/*
 * Go from pass 0 to pass 1 or 2
 */
void newPass(fn)
char *fn;
{
	register seg *s;
	unsigned size;
	i32_t xaddr;
	
	if ((cseg->curadd = dot.loc) > cseg->hiadd)
		cseg->hiadd = cseg->curadd;
	cseg = segs;	/* return to .text */
	defCt = macNo = dot.loc = 0;
	if ((txtAt = DEBUG_RECS) && !pass) {
		xpass = 1;	/* force another pass */
		symptr = syms = (struct xsym*)alloc(coffDefCt * sizeof(*syms));
	}
	wideMode = pass = dot.sg = 1;
	if (indPass()) {	/* take an extra pass */
		usects = 0;
		for (s = segs; s < segend; s++) {
			if (s->hiadd || usects < MANSEG)
				usects++;
			s->segSeq = usects;
			s->bp = s->buf;
			s->relBp = s->relBuf;
			s->lineBp = s->lineBuf;
			s->s_nlnno = s->s_nreloc = s->hiadd = s->curadd = 0;
		}
		symGlob(txtAt + USECTS); /* fix symbol table */
		coffDefCt = coffAuxCt = coffEfcnCt = 0;
		return;
	}

	pass   = 2;	/* last pass */
	linect = nlpp;
	usects = xaddr = datapos = relpos = linepos = 0;
	ofp = xopen(fn, "wb");
	
	for (s = segs; s < segend; s++) {
		if (s->hiadd || usects < MANSEG)
			usects++;
		s->s_paddr = s->s_vaddr = xaddr;
		s->hiadd += ROUNDR;		/* Round up */
		s->hiadd &= ~ROUNDR;
		xaddr += s->s_size = s->hiadd;
		s->data_seekad = datapos; /* temp value */
		s->s_relptr = relpos;
		s->s_lnnoptr = linepos;
		if (s->s_flags != STYP_BSS) {
			datapos += s->hiadd;
			relpos  += s->s_nreloc * RELSZ;
			linepos += s->s_nlnno  * LINESZ;
		}
	}
	size =  sizeof(FILEHDR) + (usects * sizeof(SCNHDR));
	debpos  = size + datapos + relpos + linepos;
	sympos  = debpos + (txtAt * SYMESZ);
	linepos = size + datapos + relpos;
	relpos  = size + datapos;

	if (fseek(ofp, lastSeek = sympos, 0))
		fatal("Seek error on object file");	/**/

	usects = 0;
	for (s = segs; s < segend; s++) {
		SYMENT sym;
		AUXENT aux;

		if (s->hiadd || usects < MANSEG)
			usects++;
		else
			continue;

		/* write symbol records for segments */
		clear(&sym);
		s->segSeq = usects;
		sym.n_sclass = C_STAT;
		sym.n_value = s->s_vaddr;
		memcpy(sym._n._n_name, s->s_name, SYMNMLEN);
		sym.n_scnum = usects;
		sym.n_numaux = txtAt ? 1 : 0;
		owrite((const char*)&sym, sizeof(sym));

		if (sym.n_numaux) {
			aux.ae_scnlen = s->s_size;
			aux.ae_nreloc = s->s_nreloc;
			aux.ae_nlinno = s->s_nlnno;
			owrite((const char*)&aux, sizeof(aux));
		}

		if (s->hiadd && (s->s_flags != STYP_BSS))
			s->s_scnptr = s->data_seekad += size;
		else
			s->s_scnptr = s->data_seekad = 0;

		if (s->s_nreloc && (s->s_flags != STYP_BSS))
			s->relo_seekad = s->s_relptr += relpos;
		else
			s->relo_seekad = s->s_relptr = 0;

		if (s->s_nlnno && (s->s_flags != STYP_BSS))
			s->line_seekad = s->s_lnnoptr += linepos;
		else
			s->line_seekad = s->s_lnnoptr = 0;
		s->hiadd = s->curadd = 0;
	}

	sympos = ftell(ofp);	/* continue the symbol table from here */

	/* fix the symbol table setting numbers */
	symGlob(txtAt + USECTS);
	coffDefCt = coffAuxCt = coffEfcnCt = 0;
}

/*
 * write record header then segment headers.
 */
static void
writeHeader()
{
	register seg *s;
	FILEHDR head;
	int i;
	union { time_t t; i32_t i[2]; } tt;

	head.f_magic = C_386_MAGIC;
	tt.i[1] = 0;  /* Defensive programming in case libc .h is 32 bits and libc code is 64 bits. */
	time(&tt.t);
	head.f_timdat = tt.t;  /* Discard high bits if shorter. */
	head.f_nsyms = symbs;
	head.f_symptr = debpos;
	head.f_opthdr = 0;
	head.f_flags = 0;
	head.f_nscns = usects;

	xwrite(0L, (char *)&head, sizeof(head));
	for (i = 0, s = segs; s < segend; s++) {
		if (s->hiadd || i < MANSEG) {
			i++;
			owrite((const char*)s, sizeof(SCNHDR));
		}
	}
}

/*
 * Finish any output.
 */
void cleanUp()
{
	int pad;
	register seg *s;

	s = cseg;		/* close out current seg */
	if(s->hiadd < (s->curadd = dot.loc))
		s->hiadd = s->curadd;

	if (coffDefCt)	/* Write debug symbols */
		writeDebug();

	fseek(ofp, sympos, 0);

	symDump(outSym, txtAt);	/* write ordinary symbols */

	symbs = (ftell(ofp) - debpos) / SYMESZ; /* figure sym ct from loc */

	if (strOff > 4) {
		owrite((const char*)&strOff, sizeof(strOff)); /* write length of tail */
		writeDebugLong();   /* dump symbols that are too long */
		symDump(outSymStr, txtAt);	
	}

	writeHeader();	/* now we know header data */

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
		outLineRec(s);
	}
}

/*
 * org command
 */
void doOrg(parm *label, data *oper)
{
	register seg *s;
	i32_t from, to;
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
		return;  /* Not reached. */
	case 'y':
		if (oper->d.y->sg != dot.sg) {
			yyerror("Org to wrong segment");
			/* You must \fB.org\fR to the current segment. */
		}
		to = oper->d.y->loc;
		break;
	case 'l':
		to = oper->d.l;	/* org where requested */
		break;
	default:
		to = 0;  /* Pacify GCC about uninitialized variable. */
	}

	if ((pass != 2) || (s->s_flags == STYP_BSS)) { /* no disk work */
		s->curadd = dot.loc = to;	/* make segment current again */
		if (to > s->hiadd)
			s->hiadd = dot.loc;
	}
	if (to <= s->hiadd) {	/* no pad if we've been there */
		s->curadd = dot.loc = to;
		s->data_seekad += to - from;
	}
	from = s->curadd = dot.loc = s->hiadd;	/* seek to the old end */
	s->data_seekad += s->hiadd - from;

	pad = (s->s_flags == STYP_TEXT) ? NOP : 0; /* pad out */
	for (;from < to; from++)
		binout(pad, s);
	s->curadd = dot.loc = to;	/* make segment current again */
	if (to > s->hiadd)
		s->hiadd = dot.loc;
}

/*
 * Initialize segment information.
 */
void segInit(void)
{
	int i;
	register seg *s;

	static char *segclass[] = {
		".text", ".data", ".bss"
	};
	static i32_t segflag[] = {
		STYP_TEXT, STYP_DATA, STYP_BSS
	};

	cseg = segs = (seg*)alloc(sizeof(*segs) * MANSEG);
	segend = segs + MANSEG;

	for (i = 0, s = segs; s < segend; s++, i++) {
		strncpy(s->s_name, segclass[i], SYMNMLEN);
		s->s_flags = segflag[i];
		s->segSeq = ++sects;
		s->bp = s->buf;
		s->relBp = s->relBuf;
		s->lineBp = s->lineBuf;
	}
}

/*
 * .ident and .version set stuff in the comment section.
 */
void cmnt(opc *op, parm *p)
{
	(void)op; (void)p;
}

/*
 * coff debugging statements.
 */
void coffFile(parm *s)
{
#ifdef NODEBUG
	return;
#else
	coffDefCt++;
	coffAuxCt++;
	if (2 == pass) {
		symptr->sclass = C_FILE;
		symptr->scnum  = N_DEBUG;
		symptr->numaux = 1;
		if ('"' == s->str[0]) {
			char *p;

			if (NULL != (p = strchr(s->str + 1, '"')))
				*p = 0;
			strncpy(symptr->aux.ae_fname, s->str + 1, FILNMLEN);
		}
		else
			strncpy(symptr->aux.ae_fname, s->str, FILNMLEN);
		symptr->name = ".file";
		symptr++;
	}
#endif
}

void coffDef(parm *s)
{
#ifdef NODEBUG
	return;
#else
	if (pass) {	/* initialize new symbol */
		if (defSw)
			yywarn("missing .endef");
		symptr->scnum = N_ABS;
		if (NULL != symptr->name)
			free(symptr->name);
		symptr->name  = scpy(s->str, 0);
		symptr->symno = DEBUG_RECS;
		defSw = 1;
	}
#endif
}

void coffEndef(void)
{
#ifdef NODEBUG
	return;
#else
	if ((1 == pass) && (symptr->scnum != N_DEBUG))
		symReNumber(symptr->name, DEBUG_RECS);

	if (2 == pass) {
		register struct xsym *s;

		switch (symptr->sclass) {
		case C_BLOCK:
			if (strcmp(symptr->name, ".eb"))
				break;
			for (s = symptr - 1; s != syms; s--)
				if (C_BLOCK == s->sclass &&
				    !strcmp(s->name, ".bb") &&
				    !s->aux.ae_endndx) {
					s->aux.ae_endndx = DEBUG_RECS + 2;
					break;
				}
			if (s == syms)
				yywarn(".eb does not connect to .bb");
				/* A .eb statement does not connect to a
				 * .bb statement */
			break;

		case C_EFCN:	/* end of function marker */
			for (s = symptr - 1; s != syms; s--) {
				if (!strcmp(symptr->name, s->name)) {
					short type;

					/* if this is a function at any level */
					for (type = s->type & ~N_BTMASK;
					     type && !ISFCN(type);
					     type >>= N_TSHIFT)
					 	;

					if (type)
						break;
				}
			}
			if (s == syms)
				yywarn(".type -1 does not connect");
				/* A type of -1 C_EFCN does not connect to
				 * a .def of a function */
			else {
				s->aux.ae_fsize = symptr->value - s->value;
				s->aux.ae_endndx = DEBUG_RECS;
			}
		}
		symptr->numaux = auxSw;
		symptr++;
		if (!defSw)
			yywarn(".endef must follow .def"); /* */
	}
	coffDefCt++;			/* we need a spot to remember this */
	coffEfcnCt += efcnSw;		/* C_EFCN don't write to table */
	if (!efcnSw)			/* They don't have aux records */
		coffAuxCt += auxSw;
	auxSw = defSw = efcnSw = 0;
#endif
}

/*
 * .type command
 */
void coffType(i32_t n)
{
#ifdef NODEBUG
	return;
#else
	if (ISFCN(n)) { /* function build a line record */
		register LINENO *bp;

		auxSw = 1;
		if (2 != pass)
			cseg->s_nlnno++;
		else {
			bp = cseg->lineBp++;
			bp->l_lnno = 0;
			bp->l_addr.l_symndx = DEBUG_RECS;
		
			if (bp == (cseg->lineBuf + (LINEBUF - 1)))
				outLineRec(cseg);
		}
	}

	if (2 == pass) {
		if (!defSw)
			yywarn(".type not in .endif");
			/* Debug command .type must be in .endif */
		symptr->type = n;
	}
#endif
}

void coffVal(data *item)
{
#ifdef NODEBUG
	return;
#else

	if (2 != pass)
		return;
	if (!defSw)
		yywarn(".val must follow .def"); /* */

	if ('y' == item->type) {
		sym *sp = item->d.y;

		if (sp->flag & S_COMMON) {
			symReNumber(symptr->name, DEBUG_RECS);
			symptr->scnum = N_UNDEF;
			symptr->value = sp->size;
		}
		else {
			symptr->scnum = sp->sg;
			symptr->value = sp->loc;
		}
	}
	else
	 	symptr->value = item->d.l;
#endif
}

void coffScl(signed char n)
{
#ifdef NODEBUG
	return;
#else
	if (C_EFCN == n)
		efcnSw = 1;
	if (!pass)
		return;
	if (!defSw)
		yywarn(".scl must follow .def"); /* */
	if (ISTAG(n))
		symptr->scnum = N_DEBUG;
	symptr->sclass = n;
#endif
}

void coffSize(i32_t n)
{
#ifdef NODEBUG
	return;
#else
	auxSw = 1;
	if (2 != pass)
		return;
	if (!defSw)
		yywarn(".size must follow .def"); /* */
	symptr->aux.ae_size = n;
#endif
}

void coffDim(i32_t n, int d)
{
#ifdef NODEBUG
	return;
#else
	auxSw = 1;
	if (2 != pass)
		return;
	if (!defSw) {
		yywarn(".dim must follow .def"); /* */
		return;
	}
	if (DIMNUM <= d) {
		yywarn(".dim statement too complex");
		/* A .dim statement may contain no more than 4 dimensions. */
		return;
	}
	symptr->aux.ae_dimen[d] = n;
#endif
}

void coffTag(parm *p)
{
#ifdef NODEBUG
	return;
#else
	struct xsym *s;

	auxSw = 1;
	if (2 != pass)
		return;
	for (s = symptr - 1; s != syms; s--) {
		if (ISTAG(s->sclass) && !strcmp(p->str, s->name)) {
		    	if (C_EOS == symptr->sclass)
				s->aux.ae_endndx = symptr->symno + 2;
			symptr->aux.ae_tagndx = s->symno;
			break;
		}
	}
	if (s == syms)
		yywarn(".tag does not connect");
		/* This tag fails to connect to an earlier unconnected .def
		 * of the same name and a proper n_sclass.
		 */
#endif
}

void coffLn(i32_t n)
{
#ifdef NODEBUG
	return;
#else
	register LINENO *bp;

	if (2 != pass)
		cseg->s_nlnno++;
	else {
		bp = cseg->lineBp++;
		bp->l_lnno = n;
		bp->l_addr.l_paddr = dot.loc;
		
		if (bp == (cseg->lineBuf + (LINEBUF - 1)))
			outLineRec(cseg);
	}
#endif
}

void coffLine(i32_t n)
{
#ifdef NODEBUG
	return;
#else
	auxSw = 1;
	if (2 != pass)
		return;
	if (!defSw)
		yywarn(".line must follow .def"); /* */
	symptr->aux.ae_lnno = n;	
#endif
}

/*
 * Write debug records.
 */
void writeDebug()
{
	register struct xsym *s, *ends;
	int ct = 0, act = 0;

	fseek(ofp, lastSeek = debpos, 0);
	for (ends = (s = syms) + coffDefCt; s != ends; s++) {
		int i;
		SYMENT sym;

		if (C_EFCN == s->sclass)
			continue;

		sym.n_sclass = s->sclass;
		sym.n_value  = s->value;
		sym.n_scnum  = s->scnum;
		sym.n_type   = s->type;
		sym.n_numaux = s->numaux;

		if ((i = strlen(s->name)) > SYMNMLEN) {
			sym.n_offset = strOff;
			sym.n_zeroes = 0;
			strOff += i + 1;
		}
		else
			strncpy(sym.n_name, s->name, SYMNMLEN);
		
		ct++;
		owrite((const char*)&sym, SYMESZ);
		if (sym.n_numaux) {
			act++;
			owrite((const char*)&(s->aux), AUXESZ);
		}
	}
	
}

void writeDebugLong()
{
	register struct xsym *s, *ends;

	for (ends = (s = syms) + coffDefCt; s != ends; s++) {
		int i;

		if (C_EFCN == s->sclass)
			continue;

		if ((i = strlen(s->name)) > SYMNMLEN)
			owrite(s->name, i + 1);
	}
}
	
/*
 * .comm and .lcomm commands.
 * Improve later.
 */
void comm(opc *op, parm *p, i32_t n)
{
	if (NULL == p) {
		yyerror(".comm must have tag");
		/* The format of \fB.comm\fR is \fB.comm name, size\fR. */
		return;
	}
	if (op->code == 2) {
		sym *s;

		s = symLookUp(p->str, S_LOCAL, 0L, op->code + 1);
		s->size = n;
		s->flag = S_EXDEF|S_COMMON;
	}
}

/*
 * segment command.
 */
void
segment(op, p, n)
const opc *op;
parm *p;
i32_t n;
{
	register seg *s;
	sym *rv;
	static int previous = 0;
	int thisSg;

	/* Implement one level segment stack */
	if (10000 != op->code) {
		thisSg = op->code;
		previous = dot.sg - 1;	/* save where we were */
	}
	else	/* .previous pops the stack */
		thisSg = previous;

	s = segs + thisSg;

	if (NULL == p) {	/* segment change */
		if ((cseg->curadd = dot.loc) > cseg->hiadd)
			cseg->hiadd = cseg->curadd;

		cseg = s;
		dot.loc = s->curadd;
		dot.sg = thisSg + 1;
		return;
	}

	/* set up data */
	rv = symLookUp(p->str, S_LOCAL, s->curadd, thisSg + 1);
	rv->size = n;
	s->curadd += n;
	if (s->curadd > s->hiadd)
		s->hiadd = s->curadd;
	return;
}

/*
 * Switch to a section by name.
 * add it if required.
 */
void
section(name)
const char *name;
{
	int motion, i;
	char *oldsegs;
	register seg *s;
	static opc segOp = { 0, S_SEGMENT };

	/* These are common segment names with their flags */
	static char *segclass[] = {
		".init", ".fini", ".rodata",
		".comment", ".ctors", ".dtors"
	};
	static i32_t segflag[] = {
		STYP_TEXT, STYP_DATA, STYP_DATA,
		STYP_INFO, STYP_DATA, STYP_DATA
	};

	/* Look for section in segment list */
	for (segOp.code = 0; segOp.code < sects; segOp.code++) {
		if (!strncmp(name, segs[segOp.code].s_name, 8)) {
			segment(&segOp, NULL, 0L);
			return;
		}
	}

	/* realloc segment list with room for one more */
	oldsegs = (char *)segs;
	if (NULL == (segs = realloc(segs, sizeof(*segs) * ++sects)))
		fatal("Out of memory"); /**/

	/* adjust pointers to segment list */
	motion = (char *)segs - oldsegs;
#define adjust(ptr) ptr = (void *)(((char *)(ptr)) + motion)
	adjust(cseg);
	adjust(segend);
	for (s = segs; s < segend; s++) {
		adjust(s->bp);
		adjust(s->relBp);
		adjust(s->lineBp);
	}
#undef adjust
	segend++;

	/* build new entry on segment list */
	memset(s, '\0', sizeof(*s));
	strncpy(s->s_name, name, SYMNMLEN);
	
	s->s_flags = STYP_TEXT;		/* set default */
	for (i = 0; i < 6; i++) {	/* look at common names */
		if (!strcmp(name, segclass[i])) {
			s->s_flags = segflag[i];
			break;
		}
	}

	s->segSeq = sects - 1;
	s->bp = s->buf;
	s->relBp = s->relBuf;
	s->lineBp = s->lineBuf;

	segment(&segOp, NULL, 0L);	/* switch to the new entry */
}
