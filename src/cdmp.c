/*
 * cdmp.c
 * 8/12/92
 * Requires libmisc functions: cc cdmp.c -lmisc
 * Read and print COFF files.
 * Usage: cdmp [ -adlrsx ] filename ...
 * Options:
 * 	-a	supress symbol aux entries
 * 	-d	supress data dumps
 *	-i	dump .text in instr mode
 * 	-l	supress line numbers
 * 	-r	supress relocation entries
 * 	-s	supress symbol entries
 *	-x	dump aux entries in hex
 * Does not know all there is to know about aux entry structure yet.
 */

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "coff.h"
#include "intsize.h"

char *alloc(size_t n);
FILE *xopen(const char *fn, const char *acs);
int getargs(int argc, char * const argv[], const char *optstring);

#define	VERSION	"V2.0"
#define VHSZ	48		/* line size in vertical hex dump */
typedef	char	SECNAME[9];	/* NUL-terminated 8 character section name */

/* Some shortcut display stuff. */
#define show(flag, msg) if (fh.f_flags & flag) printf("\t" msg "\n");
#define cs(x) case x: printf(#x); break;
#define cd(x) case x: printf(#x "\tvalue=%" PRId32 " ", se->n_value); break;
#define cx(x) case x: printf(#x "\tvalue=0x%" PRIx32 " ", se->n_value); break;

/* Externals. */
extern	char	*as_optarg;

/* Forward. */
char *checkStr(char *s);
void optHeader(void);
void readHeaders(const char *fn);
void shrLib(void);
void readSection(register int n);
void readStrings(void);
void readSymbols(void);
void print_aux(int n, register SYMENT *sep);
void print_sym(SYMENT *se, c32_t n);
void dump(register char *buf, register int p);
int clean(register int c);
void outc(register int c, register int i, register int s);
int hex(register int c);

/* Globals. */
char	aswitch;		/* Suppress aux entry dumps		*/
char	buf[VHSZ];		/* Buffer for hex dump			*/
char	dswitch;		/* Suppress data dumps			*/
char	iswitch;		/* Dump text in instr mode		*/
FILE	*fp;			/* COFF file pointer			*/
char	lswitch;		/* Suppress line number dumps		*/
c32_t	num_sections;		/* Number of sections			*/
c32_t	num_symbols;		/* Number of symbols			*/
char	rswitch;		/* Suppress reloc dumps			*/
SECNAME	*sec_name;		/* Section names			*/
c32_t	section_seek;		/* Seek to seek start of section	*/
char	sswitch;		/* Suppress symbol dumps		*/
char	*str_tab;		/* String char array			*/
c32_t	symptr;			/* File pointer to symbol table entries	*/
char	xswitch;		/* Dump aux entries in hex		*/

#ifdef __MINILIBC686__
#  define strerror strerror_few  /* Store only the few most common error messages in the executable, the rest will be reported as "?". */
#endif

/*
 * Print fatal error message and die.
 */
/* VARARGS */
void fatal(const char *fmt, ...)
{
	va_list ap;
	register int save;

	save = errno;
	fputs("cdmp: ", stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fputs("\n", stderr);
	if (0 != save) {
		fputs("errno reports: ", stderr);
		fputs(strerror(save), stderr);
		fputs("\n", stderr);
	}
	exit(1);
}

/*
 * Return a printable version of string s,
 * massaging nonprintable characters if necessary.
 */
char *checkStr(char *s)
{
	register unsigned char *p, c;
	register int ct, badct;
	static char *work = NULL;

	for (badct = 0, ct = 1, p = (unsigned char*)s; (c = *p++); ct++)
		if ((c <= ' ') || (c > '~'))
			badct += 2;	/* not printable as is */

	if (!badct)
		return s;		/* ok as is */

	if (NULL != work)
		free(work);		/* free previous */

	work = alloc(badct + ct);
	for (p = (unsigned char*)work; (c = (unsigned char)*s++);) {
		if (c > '~') {
			*p++ = '~';
			c &= 0x7f;
		}
		if (c <= ' ') {
			*p++ = '^';
			c |= '@';
		}
		*p++ = c;
	}
	return work;
}

/*
 * Process optional file header.
 */
void optHeader(void)
{
	AOUTHDR	oh;

	if (1 != fread(&oh, sizeof(oh), 1, fp))
		fatal("error reading optional header");

	printf("\nOPTIONAL HEADER VALUES\n");
	printf("magic            = 0x%x\n", oh.magic);
	printf("version stamp    = %d\n", oh.vstamp);
	printf("text size        = 0x%" PRIx32 "\n", oh.tsize);
	printf("init data size   = 0x%" PRIx32 "\n", oh.dsize);
	printf("uninit data size = 0x%" PRIx32 "\n", oh.bsize);
	printf("entry point      = 0x%" PRIx32 "\n", oh.entry);
	printf("text start       = 0x%" PRIx32 "\n", oh.text_start);
	printf("data start       = 0x%" PRIx32 "\n", oh.data_start);
}

/*
 * Process file header.
 */
void readHeaders(const char *fn)
{
	FILEHDR	fh;
	union { time_t t; i32_t i[2]; } tt;

	fp = xopen(fn, "rb");

	if (1 != fread(&fh, sizeof(fh), 1, fp))
		fatal("error reading COFF header");

	tt.i[1] = 0;  /* Defensive programming in case libc .h is 32 bits and libc code is 64 bits. */
	tt.t = fh.f_timdat;  /* In case of size difference. */
	printf("FILE %s HEADER VALUES\n", fn);
	printf("magic number   = 0x%x\n", fh.f_magic);
	printf("sections       = %" PRId32 "\n", num_sections = fh.f_nscns);
	printf("file date      = %s", ctime(&tt.t));
	printf("symbol ptr     = 0x%" PRIx32 "\n", symptr = fh.f_symptr);
	printf("symbols        = %" PRId32 "\n", num_symbols = fh.f_nsyms);
	printf("sizeof(opthdr) = %d\n", fh.f_opthdr);
	printf("flags          = 0x%x\n", fh.f_flags);
	show(F_RELFLG, "Relocation info stripped from file");
	show(F_EXEC, "File is executable");
	show(F_LNNO, "Line numbers stripped from file");
	show(F_LSYMS, "Local symbols stripped from file");
	show(F_MINMAL, "Minimal object file");

	/*
	 * Allocate section name array.
	 */
	if (num_sections != 0)
		sec_name = (SECNAME *)alloc(((int)num_sections) * (sizeof(SECNAME)));

	if (fh.f_opthdr)
		optHeader();			/* optional header */
	section_seek = sizeof(FILEHDR) + fh.f_opthdr;
}

/*
 * Process shared library.
 */
void shrLib(void)
{
	SHRLIB shr;
	register c32_t i;
	register char *pathn;

	if (1 != fread(&shr, sizeof(shr), 1, fp))
		fatal("error reading library section");

	if (shr.pathndx -= 2) {
		c32_t j;
		printf("\nExtra Library info:\n");

		for (j = shr.pathndx * 4;
		     j && (i = fread(buf, 1, ((j > VHSZ) ? VHSZ : (int)j), fp));
		     j -= i) {
			if (!i)
				fatal("unexpected EOF in .lib data");
			dump(buf, (int)i);
		}
	}

	pathn = alloc(i = (shr.entsz - 2) * 4);
	if (1 != fread(pathn, i, 1, fp))
		fatal("error reading library name");
	printf("\nReferences %s\n", pathn);
	free(pathn);
}

/*
 * Process sections.
 */
void readSection(register int n)
{
	SCNHDR	sh;
	register c32_t i;

	fseek(fp, section_seek, 0);
	if (1 != fread(&sh, sizeof(SCNHDR), 1, fp))
		fatal("error reading section header");

	section_seek += sizeof(SCNHDR);
	fseek(fp, sh.s_scnptr, 0);

	strncpy(sec_name[n], checkStr(sh.s_name), sizeof(SECNAME) - 1);
	printf("\n%s - SECTION HEADER -\n", sec_name[n]);
	printf("physical address    = 0x%" PRIx32 "\n", sh.s_paddr);
	printf("virtual address     = 0x%" PRIx32 "\n", sh.s_vaddr);
	printf("section size        = 0x%" PRIx32 "\n", sh.s_size);
	printf("file ptr to data    = 0x%" PRIx32 "\n", sh.s_scnptr);
	printf("file ptr to relocs  = 0x%" PRIx32 "\n", sh.s_relptr);
	printf("file ptr to lines   = 0x%" PRIx32 "\n", sh.s_lnnoptr);
	printf("relocation entries  = %u\n", sh.s_nreloc);
	printf("line number entries = %u\n", sh.s_nlnno);
	printf("flags               = 0x%" PRIx32 "\t", sh.s_flags);
	switch((int)sh.s_flags) {

	case STYP_GROUP:
		printf("grouped section");	break;

	case STYP_PAD:
		printf("padding section");	break;

	case STYP_COPY:
		printf("copy section");		break;

	case STYP_INFO:
		printf("comment section");	break;

	case STYP_OVER:
		printf("overlay section");	break;

	case STYP_LIB:
		printf(".lib section\n");
		shrLib();
		return;

	case STYP_TEXT:
		printf("text only");		break;

	case STYP_DATA:
		printf("data only");		break;

	case STYP_BSS:
		printf("bss only");		break;

	default:
		printf("unrecognized section");
		break;
	}
	fputc('\n', stdout);

	/* print instructions */
	if (iswitch && !strcmp(sh.s_name, ".text")) {
		char *code;

		code = alloc(sh.s_size);
		fseek(fp, sh.s_scnptr, 0);
		if (1 != fread(code, sh.s_size, 1, fp))
			fatal("Error reading .text segment");
#if 0  /* dumpInst is not implemented. */
		dumpInst(code, sh.s_size);  /* TODO(pts): Get an implementation from Coherent. */
#else
		dump(code, (int)sh.s_size);
#endif
	}
	/* Print raw data. */
	else if (!dswitch && strcmp(sh.s_name, ".bss")) { /* don't output bss */
		register c32_t j;

		fseek(fp, sh.s_scnptr, 0);
		printf("\nRAW DATA\n");

		for (j = sh.s_size;
		     j && (i = fread(buf, 1, ((j > VHSZ) ? VHSZ : (int)j), fp));
		     j -= i) {
			if (!i)
				fatal("unexpected EOF in %.8s data",
				      checkStr(sh.s_name));
			dump(buf, (int)i);
		}
	}

	/* Print relocs. */
	if (!rswitch && sh.s_nreloc) {
		fseek(fp, sh.s_relptr, 0);
		printf("\nRELOCATION ENTRIES\n");
		for (i = 0; i < sh.s_nreloc; i++) {
			RELOC	re;	/* Relocation entry structure */

			if (1 != fread(&re, RELSZ, 1, fp))
				fatal("error reading relocation entry");

			printf("address=0x%" PRIx32 "\tindex=%" PRId32 " \ttype=",
				re.r_vaddr, re.r_symndx);
			switch(re.r_type) {
			cs(R_DIR8)
			cs(R_DIR16)
			cs(R_DIR32)
			cs(R_RELBYTE)
			cs(R_RELWORD)
			cs(R_RELLONG)
			cs(R_PCRBYTE)
			cs(R_PCRWORD)
			cs(R_PCRLONG)
			cs(R_NONREL)
			default:
				fatal("unexpected relocation type 0x%x",
					re.r_type);
				break;
			}
			fputc('\n', stdout);
		}
	}

	/* Print line numbers. */
	if (!lswitch && sh.s_nlnno) {
		fseek(fp, sh.s_lnnoptr, 0);
		printf("\nLINE NUMBER ENTRIES\n");

		for (i = 0; i < sh.s_nlnno; i++) {
			LINENO	le;	/* Line number entry structure */

			if (1 != fread(&le, LINESZ, 1, fp))
				fatal("error reading line number entry");

			if (le.l_lnno)
				printf("address=0x%" PRIx32 "\tline=%d\n",
					le.l_addr.l_paddr, le.l_lnno);
			else
				printf("function=%" PRId32 "\n", le.l_addr.l_symndx);
		}
	}
}

/*
 * Read the string table into memory.
 * This allows readSymbols() to work.
 */
void readStrings(void)
{
	register unsigned char *str_ptr, c;
	c32_t str_seek;
	u32_t str_length;
	unsigned len;

	str_seek = symptr + (SYMESZ * num_symbols);
	fseek(fp, str_seek, 0);

	if (1 != fread(&str_length, sizeof(str_length), 1, fp))
		str_length = 0;

	if (str_length == 0) {
		printf("\nNO STRING TABLE\n");
		return;
	}
	printf("\nSTRING TABLE DUMP\n");
	len = str_length -= 4;
	if (len != str_length)
		fatal("bad string table length %ld", str_length);
	str_tab = alloc(len);
	if (1 != fread(str_tab, len, 1, fp))
		fatal("error reading string table %" PRIx32 " %d", ftell(fp), len);

	for (str_ptr = (unsigned char*)str_tab; (char*)str_ptr < str_tab + str_length; ) {
		fputc('\t', stdout);
		while ((c = *str_ptr++)) {
			if (c > '~') {
				c &= 0x7f;
				fputc('~', stdout);
			}
			if (c < ' ') {
				c |= '@';
				fputc('^', stdout);
			}
			fputc(c, stdout);
		}
		fputc('\n', stdout);
	}
}

/*
 * Process symbol table.
 */
void readSymbols(void)
{
	SYMENT se;
	register c32_t i, j, naux;

	if (sswitch)
		return;
	fseek(fp, symptr, 0);
	printf("\nSYMBOL TABLE ENTRIES\n");
	for (i = 0; i < num_symbols; i++) {
		if (1 != fread(&se, SYMESZ, 1, fp))
			fatal("error reading symbol entry");
		print_sym(&se, i);
		naux = se.n_numaux;
		for (j = 0; j < naux; j++)
			print_aux(i+j+1, &se);
		i += naux;
		if (i >= num_symbols)
			fatal("inconsistant sym table");
	}
}

/*
 * Process a symbol aux entry.
 * This is still pretty ad hoc, it may not do all entries correctly yet.
 * Does not print 0-valued fields.
 */
void print_aux(int n, register SYMENT *sep)
{
	AUXENT ae;
	register int type, sclass, i;
	register c32_t l;
	int has_fsize, has_fcn;
	unsigned short *sp;
	char fname[FILNMLEN + 1];

	if (1 != fread(&ae, AUXESZ, 1, fp))
		fatal("error reading symbol aux entry");
	if (aswitch)
		return;					/* suppressed */
	printf("%4d\t", n);				/* symbol number */
	if (xswitch) {					/* dump in hex */
		printf("\tAUX ENTRY DUMP\n");
		dump((char*)&ae, sizeof(ae));
		return;
	}

	sclass = sep->n_sclass;
	type = sep->n_type;

	if (sclass == C_FILE) {				/* .file */
		strncpy(fname, ae.ae_fname, FILNMLEN);
		fname[FILNMLEN] = '\0';
		printf("\tfilename=%s\n", checkStr(fname));
		return;
	} else if (sclass == C_STAT && type == T_NULL) {	/* section name */
		printf("\tlength=%" PRIx32 "\trelocs=%d\tlinenos=%d\n",
			ae.ae_scnlen,
			ae.ae_nreloc,
			ae.ae_nlinno);
		return;
	}

	/*
	 * In cases not handled above,
	 * the AUXENT is an x_sym which must be decyphered.
	 * Flags tell which members of unions to dump.
	 * The flag setting might not be quite right yet.
	 */
	has_fsize = has_fcn = 0;
	if (sclass == C_STRTAG || sclass == C_UNTAG || sclass == C_ENTAG
	 || sclass == C_BLOCK)		/* tag definitions or .bb or .eb */
		++has_fcn;
	if (ISFCN(type)) {
		++has_fsize;
		++has_fcn;
	}

	/* Print tag index. */
	if ((l = ae.ae_tagndx))
		printf("\ttag=%" PRId32 "", l);

	/* Print fsize or lnsz info. */
	if (has_fsize) {
		if ((l = ae.ae_fsize))
			printf("\tfsize=%" PRId32 "", l);
	} else {
		if ((i = ae.ae_lnno))
			printf("\tlnno=%d", i);
		if ((i = ae.ae_size))
			printf("\tsize=%d", i);
	}

	/* Print fcn or ary info. */
	if (has_fcn) {
		if ((l = ae.ae_lnnoptr))
			printf("\tlnnoptr=0x%" PRIx32 "", l);
		if ((l = ae.ae_endndx))
			printf("\tend=%" PRId32 "", l);
	} else {
		sp = ae.ae_dimen;
		if (*sp != 0) {
			printf("\tdims=< ");
			while (sp < &ae.ae_dimen[DIMNUM] && *sp)
				printf("%d ", *sp++);
			fputc('>', stdout);
		}
	}

	/* Print tv index. */
	if ((l = ae.ae_tvndx))
		printf("\ttv=%" PRId32 "", l);

	fputc('\n', stdout);
}

/*
 * Process symbol table entry.
 */
void print_sym(SYMENT *se, c32_t n)
{
	register int i, c;
	int eflag, derived;
	
	if (se->n_sclass == C_FILE && n > 0)
		fputc('\n', stdout);			/* for readability */
	printf("%4" PRId32 "   ", n);			/* index number */

	eflag = 0;				/* no errors */
	if (se->n_zeroes != 0) {		/* name in place */
		for (i = 0; i < SYMNMLEN; i++) {
			if ((' ' < (c = se->n_name[i])) && ('~' >= c))
				fputc(c, stdout);
			else {
				eflag = c;
				break;
			}
		}
		fputc('\t', stdout);
	} else					/* name in string table */
		printf("%s ", checkStr(str_tab + se->n_offset - 4));

	/* Print section. */
	i = se->n_scnum;
	printf("section=");
	if (i >= 1 && i <= num_sections)
		printf("%s", sec_name[i-1]);
	else
		switch(i) {
		cs(N_UNDEF)
		cs(N_ABS)
		cs(N_DEBUG)
		default:
			printf("%d?", i);
			break;
		}

	/* Print the type. */
	printf("\ttype=");
	i = se->n_type;
	derived = 0;
	while (i & N_TMASK) {			/* derived type */
		if (derived == 0) {
			derived = 1;
			fputc('<', stdout);
		}
		switch(i & N_TMASK) {
		cs(DT_PTR)
		cs(DT_FCN)
		cs(DT_ARY)
		case DT_NON:
		default:
			fatal("unexpected derived type 0x%x", i & N_TMASK);
		}
		fputc(' ', stdout);
		i >>= N_TSHIFT;
	}
	switch (c = (se->n_type & N_BTMASK)) {	/* base type */

	case T_NULL:
		printf("none");
		break;

	cs(T_CHAR)
	cs(T_SHORT)
	cs(T_INT)
	cs(T_LONG)
	cs(T_FLOAT)
	cs(T_DOUBLE)
	cs(T_STRUCT)
	cs(T_UNION)
	cs(T_ENUM)
	cs(T_MOE)
	cs(T_UCHAR)
	cs(T_USHORT)
	cs(T_UINT)
	cs(T_ULONG)

	case T_ARG:		/* What has base type (not storage class) ARG? */
	default:
		fatal("unexpected base type 0x%x", c);

	}
	if (derived)
		fputc('>', stdout);

	/* Print the storage class. */
	printf("\tclass=");
	switch (i = se->n_sclass) {

	cd(C_EFCN)
	cd(C_NULL)
	cd(C_AUTO)
	cx(C_STAT)
	cd(C_REG)
	cd(C_EXTDEF)
	cd(C_LABEL)
	cd(C_ULABEL)
	cd(C_MOS)
	cd(C_ARG)
	cd(C_STRTAG)
	cd(C_MOU)
	cd(C_UNTAG)
	cd(C_TPDEF)
	cd(C_ENTAG)
	cd(C_MOE)
	cd(C_REGPARM)
	cd(C_FIELD)
	cd(C_BLOCK)
	cd(C_FCN)
	cd(C_EOS)
	cd(C_FILE)

	case C_EXT:
		if (se->n_scnum != N_UNDEF)
			printf("C_EXT\tvalue=0x%" PRIx32 "", se->n_value);
		else if (se->n_value != 0)
			printf("Common\tlength=%" PRId32 "", se->n_value);
		else
			printf("External");
		break;

	case C_USTATIC:			/* What is an undefined static? */
		fatal("unexpected storage class 0x%x", i);
		/* fallthrough */

	default:
		printf("0x%x", i);

	}

#if	0
	if (se->n_numaux)
		printf("\tnaux=%d", se->n_numaux);
#endif
	fputc('\n', stdout);

	if (eflag) {
		printf("*** Bad data in name **\n");
		dump((char*)se, SYMESZ);
	}
}

/*
 * Vertical hex dump of p bytes from buffer buf.
 */
void dump(register char *buf, register int p)
{
	register int i;

	/* Offset. */
	printf ("\n%6lx\t", ftell(fp) - p);

	/* Printable version of character. */
	for (i = 0; i < p; i++ )
		outc(clean(buf[i]), i, ' ');
	printf("\n\t");

	/* High hex digit. */
	for (i = 0; i < p; i++)
		outc(hex((buf[i] >> 4) & 0x0f), i, '.');
	printf("\n\t");

	/* Low hex digit. */
	for (i = 0; i < p; i++)
		outc(hex(buf[i]& 0x0f), i, '.');
	fputc('\n', stdout);
}

/*
 * Return c if printable, '.' if not.
 */
int clean(register int c)
{
	return (c >= ' ' && c <= '~' ) ? c : '.';
}

/*
 * Print c, preceded by s every 4 times.
 */
void outc(register int c, register int i, register int s)
{
	if ((i&3) == 0 && i != 0 )
		fputc(s, stdout);
	fputc(c, stdout);
}

/*
 * Convert hex digit c to corresponding ASCII character.
 */
int hex(register int c)
{
	return ( c <= 9 ) ? c + '0' : c + 'A' - 10;
}

/*
 * Mainline.
 */
int main(int argc, char *argv[])
{
	register int i, c;

	while (EOF != (c = getargs(argc, argv, "adlrsxV?"))) {
		switch (c) {

		case 0:
			/* Process a COFF file. */
			readHeaders(as_optarg);
			for (i = 0; i < num_sections; i++)
				readSection(i);
			if (num_symbols) {
				readStrings();
				readSymbols();
			}
			/* Cleanup. */
			if (sec_name != NULL) {
				free(sec_name);
				sec_name = NULL;
			}
			if (str_tab != NULL) {
				free(str_tab);
				str_tab = NULL;
			}
			fclose(fp);
			break;

		case 'a':	aswitch++;	break;
		case 'd':	dswitch++;	break;
		case 'i':	iswitch++;	break;
		case 'l':	lswitch++;	break;
		case 'r':	rswitch++;	break;
		case 's':	sswitch++;	break;
		case 'x':	xswitch++;	break;

		case 'V':
			fprintf(stderr, "cdmp: %s\n", VERSION);
			break;

		case '?':
		default:
			fprintf(stderr,
				"Usage: cdmp [ -adlrsx ] filename ...\n"
				"Options:\n"
				"\t-a\tsupress symbol aux entries\n"
				"\t-d\tsupress data dumps\n"
				"\t-l\tsupress line numbers\n"
				"\t-r\tsupress relocation entries\n"
				"\t-s\tsupress symbol entries\n"
				"\t-x\tdump aux entries in hex\n");
			exit(1);
			break;
		}
	}
	return 0;
}

/* end of cdmp.c */
