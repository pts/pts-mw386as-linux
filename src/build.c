/*
 * 80386 Assembler Build output code.
 */
#include <asm.h>
#include <asflags.h>
#include <y_tab.h>
#include <symtab.h>

static symt *st;
static struct expr *opList[3];
static int ct;

/* instruction parts */
static expr *addr, *displ, *immed, *immedx;
static char mod, rm, reg, scale, index, base, immed8;

static unsigned long uflags;
#define U_RM	1
#define U_REG	2
#define U_IMM8	4
#define U_IMM16	8
#define U_IMM16X 0x10
#define U_IMM32	 0x20
#define U_IMM32X 0x40
#define U_REL8	0x80
#define U_REL16	0x100
#define U_ADR16 0x200
#define U_ADR32 0x400
#define U_DSP8	0x800
#define U_DSP16 0x1000
#define U_DSP32 0x2000
#define U_RELI	0x4000
#define U_CTL	0x8000L
#define U_SEG	0x10000L
#define U_FP	0x20000L

/*
 * Build indefinate opcode. On 80386 thats everything.
 * First try all instrs not in the wrong mode.
 * Then try the instrs in the wrong mode.
 */
buildind(label, op, oper)
parm *label;
register opc *op;
register expr *oper;
{
	opc *save;
	int i;

	buildlab(label);

	if (lflags & A_LONG && lflags & A_SHORT)
		yyerror("Mixed 386/286 addressing modes");
		/* No opcode allows mixed 286 and 386 addressing modes. */

	ct = countList((parm *)oper);

	if (ct > 3) {
		yyerror("Too many operands");
		/* No 386 opcode has more than three operands. */
		return (1);
	}

	if (fswitch)	/* reverse operand order */
		for (i = ct; i--; oper = oper->next)
			opList[i] = oper;
	else		/* normal operand order */
		for (i = 0; i < ct; i++, oper = oper->next)
			opList[i] = oper;

	/* try the stuff in the right mode */
	for (save = op; -1 != op->code; op++) {
		st = typTab + op->kind;
		if (longMode) {
			if (st->bldr & WORD_MODE)
				continue;
		}
		else {
			if (st->bldr & LONG_MODE)
				continue;
		}
		if(!buildop(op))
			return(0);
	}

	/* now try the others */
	for (op = save; -1 != op->code; op++) {
		st = typTab + op->kind;
		if (longMode) {
			if (!(st->bldr & WORD_MODE))
				continue;
		}
		else {
			if (!(st->bldr & LONG_MODE))
				continue;
		}
		if(!buildop(op))
			return(0);
	}

	yyerror("Illegal combination of opcode and operands");
	/* Although the opcode is valid and the operands are valid,
	 * there is no form of this opcode which takes this combination
	 * of operands in this order. */
	return(1);
}

/*
 * Check if operator validly fits mode.
 * return 1 for false zero for true.
 */
static
checkop(this, type)
register expr *this;
unsigned short type;
{
	register sym *r1;
	long d;

	r1 = this->r1;

	switch (type) {		/* preprocess rm* types */
	case rm8:
		uflags |= U_RM;
		if (T_R == this->mode) {
			if (r1->size != 1)
				return (1);
			mod = 3;
			rm = r1->loc;
			return (0);
		}
		if (lflags & A_SHORT)
			type = rm16;
		else
			type = rm32;
		break;

	case rm16:
	case m16:	/* m16 is like rm16 but can't be reg */
		uflags |= U_RM;
		if (T_R == this->mode) {
			if ((r1->size != 2) || (type == m16))
				return (1);
			mod = 3;
			rm = r1->loc;
			return (0);
		}
		if (lflags & A_SHORT)
			type = rm16;
		else
			type = rm32;
		break;

	case rm32:
	case m32:	/* m32 is like rm32 but can't be reg */
		uflags |= U_RM;
		if (T_R == this->mode) {
			if ((r1->size != 4) ||
			    (r1->flag != ORD_REG) ||
			    (type == m32))
				return (1);
			mod = 3;
			rm = r1->loc;
			return (0);
		}
		if (lflags & A_SHORT)
			type = rm16;
		else
			type = rm32;
	}

	switch(type) {
	case rel8:	/* near branch */
		uflags |= U_REL8;
		return (T_D != this->mode);
		    
	case rel16:	/* medium or long branch */
		uflags |= U_REL16;
		return (T_D != this->mode);
		    
	case mem32:	/* 32 bit simple address */
		uflags |= U_ADR32;
		rm = 5;
		return (T_D != (addr = this)->mode);

	case reli:	/* near branch */
		if (!(lflags & A_INDIR)) {
			uflags |= U_RELI;
			return (T_D != this->mode);
		}
		uflags |= U_REG;

	case rm32:	/* r/m 32 See Tables 17-3 and 17-4 */
		switch (this->mode) {

		case T_D:	/* all 32 bit disp must be good */
			mod = 0;
			rm = 5;
			uflags |= U_DSP32;
			displ = this;
			return (0);

		case T_RID:
			if ((NULL != this->ref) ||
			    (d = this->exp) < -128 || d > 127) {
				uflags |= U_DSP32;
				mod = 2;
			}
			else {
				uflags |= U_DSP8;
				mod = 1;
			}

			if (4 == (rm = r1->loc)) { /* disp (%esp) */
				base = 4;	/* base = %esp */
				index = 4;	/* no index */
			}
			displ = this;
			return (0);

		case T_R:	/* eax | ecx || edx || ebx || esi || edi */
			rm  = r1->loc;
			mod = 3;
			return(0);

		case T_RI:
			switch (rm = r1->loc) {
			case 5: /* ( %ebp ) */
				mod = 1;	/* 0 ( %ebp ) */
				uflags |= U_DSP8; /* force displacment 0 */
				displ = this;
				break;
			case 4: /* ( %esp ) */
				base = 4;	/* %sp */
				index = 4;	/* no index */
			default: /* (eax | ecx | edx | ebx | esi | edi) */
				mod = 0;
			}

			return (0);

		case T_RIS:
			if (4 == (index = r1->loc)) /* can't index %esp */
				return (1);

			rm = 4;		/* use sib */
			mod = 0;	/* no disp */
			base = 5;	/* no base */
			uflags |= U_DSP32;
			scale = this->scale;
			index = r1->loc;
			displ = this;
			return (0);

		case T_RIX:		
		case T_RIXS:
			/* can't index esp */
			if (4 == (index = this->r2->loc))
				return(1);

			if (5 != (base = r1->loc)) {
				mod = 0;
				rm = 4;
				scale = this->scale;
				return (0);
			} /* if base %ebp use T_RIXDS */

		case T_RIXD:
		case T_RIXDS:
			/* can't index esp */
			if (4 == (index = this->r2->loc))
				return (1);

			base = r1->loc;
			if ((NULL != this->ref) ||
			    (d = this->exp) < -128 || d > 127) {
				uflags |= U_DSP32;
				mod = 2;
			}
			else {
				uflags |= U_DSP8;
				mod = 1;
			}

			rm = 4;
			scale = this->scale;
			displ = this;
			return (0);

		case T_RIDS:
			if (4 == (index = r1->loc))	/* can't index sp */
				return (1);

			mod = 0;
			uflags |= U_DSP32;
			scale = this->scale;
			rm = 4;
			base = 5;
			displ = this;
			return (0);
		}
		return (1);

	case mem16:	/* 16 bit simple address */
		uflags |= U_ADR16;
		rm = 6;
		return (T_D != (addr = this)->mode);

	case rm16:	/* r/m 16 */
		switch (this->mode) {
		case T_RI:	/* register indirect */
			switch ((int)r1->loc) {
			case 6: /* (%si) */
				rm = 4;	break;
			case 7:	/* (%di) */
				rm = 5; break;
			case 3: /* (%bx) */
				rm = 7; break;
			default:
				return (1);
			}
			mod = 0;
			return (0);

		case T_R:	/* register */
			rm = r1->loc;
			mod = 3;
			return (0);
			
		case T_D:	/* displacment */
			if (this->exp < -32768L || this->exp > 65535L)
				return(1);

			mod = 0;
			rm  = 6;
			uflags |= U_DSP16;
			displ = this;
			return (0);

		case T_RID:	/* register indirect displacment */
			if (this->exp < -32768L || this->exp > 65535L)
				return(1);

			switch ((int)r1->loc) {
			case 6: /* (%si) */
				rm = 4;	break;
			case 7:	/* (%di) */
				rm = 5; break;
			case 5:	/* (%bp) */
				rm = 6; break;
			case 3: /* (%bx) */
				rm = 7; break;
			default:
				return (1);
			}

			if ((NULL != this->ref) ||
			    (d = this->exp) < -128 || d > 127) {
				uflags |= U_DSP16;
				mod = 2;
			}
			else {
				uflags |= U_DSP8;
				mod = 1;
			}
			displ = this;
			return (0);

		case T_RIXD:	/* register index displacment */
			if ((NULL != this->ref) ||
			    (d = this->exp) < -128 || d > 127) {
				uflags |= U_DSP16;
				mod = 2;
			}
			else {
				uflags |= U_DSP8;
				mod = 1;
			}
			/* fall through */
			displ = this;

		case T_RIX:	/* register index */
			if (T_RIX == this->mode)
				mod = 0;

			switch ((int)r1->loc) {
			case 3: /* %bx */
				switch ((int)this->r2->loc) {
				case 6: /* %si */
					rm = 0; break;
				case 7: /* %di */
					rm = 1; break;
				default:
					return (1);
				}
				break;
			case 5:	/* bp */
				switch ((int)this->r2->loc) {
				case 6: /* %si */
					rm = 2; break;
				case 7: /* %di */
					rm = 3; break;
				default:
					return (1);
				}
				break;
			default:
				return (1);
			}
			return (0);
		}
		return (1);

	case imm8:
		uflags |= U_IMM8;
		immed8 = this->exp;
		return (this->ref != NULL ||
			this->mode != T_IMM ||
			this->exp < -128 ||
			this->exp > 255);

	case imm8s:
		uflags |= U_IMM8;
		immed8 = this->exp;
		return (this->ref != NULL ||
			this->mode != T_IMM ||
			this->exp < -128 ||
			this->exp > 127);

	case imm16x:
		uflags |= U_IMM16X;
		d = (immedx = this)->exp;
		return (this->mode != T_IMM ||
			d < -32768L ||
			d > 65535L);

	case imm16:
		uflags |= U_IMM16;
		d = (immed = this)->exp;
		return (this->mode != T_IMM ||
			d < -32768L ||
			d > 65535L);

	case imm32x:
		uflags |= U_IMM32X;
		immedx = this;
		return (this->mode != T_IMM);

	case moffs:
		uflags |= U_IMM32;
		immed = this;
		return (this->mode != T_D);

	case imm32:
		uflags |= U_IMM32;
		immed = this;
		return (this->mode != T_IMM);

	case con1:
		return (this->mode != T_IMM ||
			this->exp != 1);

	case con3:
		return (this->mode != T_IMM ||
			this->exp != 3);

	case al:
		return (this->mode != T_R ||
			r1->flag != ORD_REG ||
			r1->size != 1 ||
			r1->loc != 0);

	case ax:
		return (this->mode != T_R ||
			r1->flag != ORD_REG ||
			r1->size != 2 ||
			r1->loc != 0);

	case eax:
		return (this->mode != T_R ||
			r1->flag != ORD_REG ||
			r1->size != 4 ||
			r1->loc != 0);

	case r16:
		if (this->mode != T_R || r1->flag != ORD_REG || r1->size != 2)
			return (1);
		uflags |= U_REG;
		reg = r1->loc;
		return (0);

	case atdx:
		if (this->mode != T_RI || r1->flag != ORD_REG ||
		    r1->size != 2 || r1->loc != 2)
		 	return(1);
		lflags &= ~A_SHORT;
		return(0);

	case cl:
		return (this->mode != T_R ||
			r1->flag != ORD_REG ||
			r1->size != 1 ||
			r1->loc != 1);

	case ds:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 3);

	case es:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 0);

	case ss:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 2);

	case fs:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 4);

	case gs:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 5);

	case cs:
		return (this->mode != T_R ||
			r1->flag != SEG_REG ||
			r1->loc != 1);

	case sreg:
		if (this->mode != T_R || r1->flag != SEG_REG)
			return (1);
		uflags |= (U_REG|U_SEG);
		reg = r1->loc;
		return (0);

	case st0:
		if (this->mode != T_FP || this->exp)
			return (1);
		uflags |= U_FP;
		return (0);

	case fpreg:
		if (this->mode != T_FP)
			return (1);
		uflags |= U_FP;
		reg = this->exp;
		return (0);

	case ctlreg:
		if (this->mode != T_R || r1->flag != CTL_REG)
			return (1);
		uflags |= U_CTL;
		rm = r1->loc;
		return (0);

	case dbreg:
		if (this->mode != T_R || r1->flag != DEB_REG)
			return (1);
		uflags |= U_CTL;
		rm = r1->loc;
		return (0);

	case treg:
		if (this->mode != T_R || r1->flag != TST_REG)
			return (1);
		uflags |= U_CTL;
		rm = r1->loc;
		return (0);

	case r32:
		if (this->mode != T_R || r1->flag != ORD_REG || r1->size != 4)
			return (1);
		uflags |= U_REG;
		reg = r1->loc;
		return (0);

	case r8:
		if (this->mode != T_R || r1->flag != ORD_REG || r1->size != 1)
			return (1);
		uflags |= U_REG;
		reg = r1->loc;
		return (0);
	}
}

/*
 * Chip errata message.
 */
errata()
{
	yywarn("This code may not work the same way on all chips");
	/* Some chips may not execute this code as expected. */
}

/*
 * Try to build an opcode.
 */
static
buildop(op)
opc *op;
{
	register unsigned short i, j;
	static short postSw = 0;
	static short lastOp = 0;
	static short lastFlags = 0;

	/* First check if everything is ok */
	if (st->operands != ct)
		return(1);

	uflags = base = mod = rm = reg = scale = index = 0;
	for (i = 0; i < ct; i++)
		if (checkop(opList[i], (unsigned short)(st->ap[i])))
			return(1);

	/* deal with unusual stuff */
	if (st->bldr & (AMBIG_MATCH | TWO_OP_MULT | XTENDS)) {
		if (st->bldr & AMBIG_MATCH)
			yywarn("Ambiguous operand length, %d bytes selected", 
			   (MOV_BYTE == op->code) ? 1 : (longMode ? 4 : 2));
			/* The assembler cannot tell the operand length by
			 * looking at the opcode and the operands.
			 * You may want to do something like change
			 * \fBmov\fR to \fBmovl\fR. */

		/* 2 operand form of 3 operand multiply */
		if (st->bldr & TWO_OP_MULT) {
			mod = 3;
			rm  = opList[1]->r1->loc;
		}

		/* movsx and movzx have mixed 16 and 32 bit stuff */
		if (st->bldr & XTENDS)
			lflags &= ~(O_LONG|O_SHORT);
	}

	if (lflags & O_LONG && lflags & O_SHORT)
		yyerror("Mixed 386/286 data modes");
		/* No 386 opcode allows mixed 286 and 386 data modes. */

	/*
	 * Only a few instructions are defined after a rep  or lock
	 * Instructions valid after lock are marked but are
	 * only valid if a memory location is accessed. This is
	 * checked by excluding (mod == 3) which is rm is register.
	 */
	if (postSw) {
		if (postSw & REP_INSTR)
			if (!(st->bldr & AFTER_REP))
				yywarn("Improper instruction following rep");
				/* Only a few instructions
				 * are valid after a rep instruction.
				 * See your machine documentation for details.*/
			else if (op->code == INSB || op->code == INSW)
				errata();

		if ((postSw & LOCK_OP) &&
		    (!(st->bldr & AFTER_LOCK) || (3 == mod)))
			yywarn("Improper instruction following lock");
			/* Only a few instructions
			 * are valid after a lock instruction.
			 * See your machine documentation for details. */
	}
	postSw = st->bldr & (LOCK_OP | REP_INSTR);

	/* 
	 * check for various chip errata
	 * sometimes wave a dead chicken over your head to make things work
	 */

#if 0
	/* See Intel chip errata for 80386-B1 17.
	 * Coprocessor instruction crossing segment boundaries may hang chip.
	 * Assume any 4's boundary is a potential boundary. */
	if ((st->bldr & FLOAT_ESC) &&
	    (((st->bldr & FLOAT_PFX) ? 2 : 3) == (dot.loc % 4)))
		if (nswitch)
			errata();
		else
			outab(NOP);
#endif

	/* See Intel chip errata for 80386-B1 23. */
	if (((lastOp == POPA) && (uflags & U_RM) && (mod != 3)) &&
		/* determine longmode of popa */
	    ((longMode ? !(lastFlags & 2) : (lastFlags & 4)) ?
		/* longmode then if base index and either not %eax */
	     ((rm == 4) && (index || base)) :
		/* not longmode any index was %eax */
	     (!rm || ((rm == 4) && (!index || !base))))) {
		if (nswitch)
			errata();
		else
			outab(NOP);
	}

	if (POP_MEM == op->code) {
		/* pop	%cs:mem */
		if (opList[0]->sg == 1)
			errata();

		/* pop 	n(%esp)	 */
		if ((uflags & U_RM) && base == 4 && rm == 4 && mod)
			errata();
	}
	
	/*
	 * aam must be preceeded with special stuff on 80486
	 * The idea is that there must be an xchg with a non 1 value.
	 */
	if (op->code == AAM) {
		static char seq[8] = {
			0x51,		/* push		%ecx */
			0x33, 0xC9,	/* xor		%ecx, %ecx */
			0x87, 0xC9,	/* xchg		%ecx, %ecx */
			0xD4, 0x0A,	/* aam */
			0x59		/* pop		%ecx */
		};

		if (nswitch)
			errata();
		else {
			for (i = 0; i < 8; i++)
				outab(seq[i]);
			return (0);
		}
	}
			
	lastFlags = st->bldr;
	lastOp = op->code;

	if (lflags & A_INDIR) {
		lastFlags = (longMode ? LONG_MODE : WORD_MODE) | MODRM_BYTE;
		switch (lastOp) {
		case JMP_NEAR:
			lastOp = JMP_INDIR;	break;
		case CALL_NEAR:
			lastOp = CALL_INDIR;	break;
		default:
			yyerror("Indirect mode on invalid instruction");
			/* Indirection is only allowed on call and jump near
			 * instructions. */
		}
	}

	if (longMode) {
		if (lflags & A_SHORT) {
			yywarn("16 bit addressing mode used in 32 bit code");
			/* You probably don't want to do this.
			 * For example, you may want to say \fB(%esi)\fR, not
			 * \fB(%si)\fR. */
			outab(PREFIX_AD);	/* address size prefix */
		}
		else
			lflags |= A_LONG;

		if (lastFlags & WORD_MODE)
			outab(PREFIX_OP);	/* operand size prefix */
	}
	else {
		if (lflags & A_LONG) {
			yywarn("32 bit addressing mode used in 16 bit code");
			/* You probably don't want to do this.
			 * For example, you may want to say \fB(%si)\fR, not
			 * \fB(%esi)\fR. */
			outab(PREFIX_AD);	/* address size prefix */
		}
		else
			lflags |= A_SHORT;

		if (lastFlags & LONG_MODE)
			outab(PREFIX_OP); /* operand size prefix */
	}

#define ck(x, y) if (j & x) break; j |= x; outab(y); break;

	/* Put out nessisary prefix bytes */
	for (j = i = 0; i < ct; i++) {
		switch (opList[i]->sg) {
		case 0:	/* es: */
			ck(1, PREFIX_ES);
		case 1: /* cs: */
			ck(2, PREFIX_CS);
		case 2: /* ss: */
			ck(4, PREFIX_SS);
		case 3: /* ds: */
			ck(8, PREFIX_DS);
		case 4: /* fs: */
			ck(16, PREFIX_FS);
		case 5: /* gs: */
			ck(32, PREFIX_GS);
		}
	}

#undef ck

	/* Then build the op code */
	if (uflags & U_REL16) {	/* 16 or 32 bit branch */
		indBra(lastOp, -1, opList[0]);
		return(0);
	}
	if (uflags & U_REL8) {	/* 8 bit branch */
		indBra(-1, lastOp, opList[0]);
		return(0);
	}
	if (uflags & U_RELI) {	/* may become 8, 16 or 32 bit branch */
		switch (lastOp) {
		case JMP_NEAR:
			indBra(lastOp, JMP_SHORT, opList[0]);
			break;
		case CALL_NEAR:
			indBra(lastOp, -1, opList[0]);
			break;
		default:	/* conditional jump */
			indBra(lastOp + JCC_NEAR,
			       lastOp + JCC_SHORT, opList[0]);
		}
		return(0);
	}

	if (lastFlags & PFX_0F)
		outab(0x0F);

	if (lastFlags & FLOAT_PFX)
		outab(0x9B);

	if (lastFlags & MODRM_BYTE ||
	    lastOp & 0xFF00)
		outab(lastOp >> 8);

	j = lastOp & 0xFF;

	if (lastFlags & ADD_REG) {
		uflags &= ~U_REG;
		j += reg;
	}
	if (lastFlags & MODRM_BYTE) {
		uflags |= U_REG;
		reg = j;
	}
	else
		outab(j);

	if (uflags & (U_RM|U_REG))
		if (lflags & A_LONG)
			outrm32();
		else
			outrm16();

	if (uflags & U_IMM16)
		outrw(immed, 0);

	if (uflags & U_IMM32)
		outrl(immed, 0);

	if (uflags & U_IMM8)
		outab(immed8);

	if (uflags & U_IMM16X)
		outrw(immedx, 0);

	if (uflags & U_IMM32X)
		outrl(immedx, 0);

	if (uflags & U_ADR16)
		outrw(addr, 0);

	if (uflags & U_ADR32)
		outrl(addr, 0);

	return(0);
}

/*
 * Output mod/rm byte and maybe sib
 */
static
outrm32()
{
	short modrm, sib;

	if (uflags & U_CTL)	/* Special register used */
		modrm = (3 << 6) | (rm << 3) | reg;
	else
		modrm = (mod << 6) | (reg << 3) | rm;

	outab(modrm);
	if (4 == rm && 3 != mod) {
		sib = (scale << 6) | (index << 3) | base;
		outab(sib);
	}

	if (uflags & U_DSP8)
		outrb(displ, 0);

	else if (uflags & U_DSP32)
		outrl(displ, 0);
}

/*
 * Output mod/rm byte
 */
static
outrm16()
{
	short modrm;

	modrm = (mod << 6) | (reg << 3) | rm;
	outab(modrm);

	if (uflags & U_DSP8)
		outrb(displ, 0);

	else if (uflags & U_DSP16)
		outrw(displ, 0);
}

/*
 * Code for relative branches.
 * Save type of all branch operators on a list assuming shortest feasable.
 * If a type changes set xpass = 1.
 *
 * Pass logic in newPass goes to 2 only if xpass == 0 else it goes to 1
 *
 * There is an elegant algorithm for fixing up jumps between passes by
 * tree manipulation, this would reduce this to a two pass assembler.
 * Sadly it won't work. It assumes smooth code, that is if I change a
 * byte jump to a near jump the following addresses will change by addition.
 * In assembly language people can insert things like .align or .org which
 * break that assumption, the GNU compiler does this every few lines.
 *
 * Once the smooth code assumption is broken we no longer know that the
 * tree algorithm terminates at all, a byte jump can go to a longer jump
 * and back again in the next pass. To guarantee termination we start at
 * byte jumps and only go to longer jumps when we know it is forced. Once
 * we go to longer jump we never go back. This speeds the assembly of GNU
 * output by about 10 times.
 */
static char xpass;		/* set if a branch changes size */
static unsigned braCt;		/* count of branches */

#define BYTE_J	0	/* byte jump length */
#define NEAR_J	1	/* int  jump length */
#define EXT_J	2	/* jump around sequence */

/*
 * Called at new pass or init. Returns 1 if another pass required.
 */
indPass()
{
	braCt = 0;		/* so far no branches */
	if (xpass) {
		xpass = 0;
		return (1);
	}
	return (0);
}

/*
 * Put out op code.
 */
static void
putOp(opCode)
register unsigned short opCode;
{
	if (opCode & 0xFF00) {
		outab(opCode >> 8);
		outab(opCode & 0xff);
	}
	else
		outab(opCode);
}

/*
 * Called for each relative branch.
 * Calculates branch size. Forces another pass if a branch expands.
 */
void
indBra(nearOp, byteOp, op)
unsigned short nearOp, byteOp;
register expr *op;
{
	static char *list;		/* one for each relative branch */
	static unsigned max;		/* size of list */
	char size;			/* BYTE_J NEAR_J EXT_J */
	long d;				/* displacment */
	short  flag, exref;
	char *old;

	/* insure space for branch data */
	if (max <= ++braCt)
		expand(&list, &max, 64, sizeof(char));

	old = list + (braCt - 1);
	/* assume size from last pass or shortest size for this jump. */
	size = pass ? *old : ((byteOp == -1) ? NEAR_J : BYTE_J);

	if (NULL == op->ref)
		fatal("NULL address in relative branch"); /* TECH */

	flag = op->ref->flag;
	exref = 0; 

	if (flag & S_UNDEF) {	/* undefined symbol */
		if (pass)
			size = NEAR_J;	/* known near */
		else if (BYTE_J == size)
			xpass = 1;	/* we must try again */

		if (gswitch)	/* -g turns undefined to global */
			exref = 1;
	}

	else if ((flag & S_EXREF) || (dot.sg != op->ref->sg)) {
		exref = 1;
		size = NEAR_J;		/* known near */
	}

	else if (BYTE_J == size) {
		/* Calculate displacment from end of byte instr */
		d = op->exp - (dot.loc + ((byteOp & 0xFF00) ? 3 : 2));

		if ((d < -128) || (d > 127))	/* near limits */
			size = NEAR_J;
	}

	/* near branch and none available build jumpover */
	if ((NEAR_J == size) && (-1 == nearOp))
		size = EXT_J;

	/* How does this compare to the last time? */
	if (*old != size) {
		switch(pass) {
		case 1:
			if (*old > size)	/* never shrink */
				break;
			xpass = 1;		/* take one more pass */
		case 0:
			*old = size;		/* take new size */
			break;
		default:
			if (*old < size)	/* too late for changes */
				fatal("Internal error relative branch logic");
				 /* TECH */
		}
	}

	/* output code */
	switch(*old) {
	case BYTE_J:	/* short op */
		putOp(byteOp);
		if (exref)
			outrb(op, 1);
		else
			outab((int)d);
		break;

	case EXT_J:	/* jump around sequence */
		putOp(byteOp);		/* caller's jump over byte jump */
		outab(2);

		outab(JMP_SHORT);	/* byte jump over near jump */
		outab(longMode ? 0x05 : 0x03);

		nearOp = JMP_NEAR;	/* near jump to caller's destination */

	case NEAR_J:	/* near jumps */
		putOp(nearOp);
		if (longMode)
			if (exref)
				outrl(op, 1);
			else	/* displacement from end of address */
				outal(op->exp - (dot.loc + 4));
		else
			if (exref)
				outrw(op, 1);
			else	/* displacement from end of address */
				outaw((int)(op->exp - (dot.loc + 2)));
	}
}
