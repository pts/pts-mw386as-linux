	.ttl	Full test of asm 386
	.llen	100
abc:
	aaa		/ 0000 0037
	aad		/ 0000 D50A
	aam		/ 0000 D40A
	aas		/ 0000 003F
	arpl	%bx, 0x3e8	/ 0000 0063
	call	abc	/ 0000 00E8
	cbw		/ 0002 0098
	cbtw		/ 0002 0098
	cwde		/ 0004 0098
	cwtl		/ 0004 0098
	clc		/ 0000 00F8
	cld		/ 0000 00FC
	cli		/ 0000 00FA
	clts		/ 0020 0006
	cmc		/ 0000 00F5
	cmpsb		/ 0800 00A6
	cmpsw		/ 0802 00A7
	cmpsl		/ 0804 00A7
	cwd		/ 0002 0099
	cwtd		/ 0002 0099
	cdq		/ 0004 0099
	cltd		/ 0004 0099
	daa		/ 0000 0027
	das		/ 0000 002F
	enter	$0x64, $0x3e8	/ 0000 00C8
	f2xm1		/ 0000 D9F0
	fabs		/ 0000 D9E1
	fbld	0x64(%di)	/ 0008 DF04
	fbstp	(%bp, %si)	/ 0008 DF06
	fchs		/ 0000 D9E0
	fcompp		/ 0000 DED9
	fcos		/ 0000 D9FF
	fdecstp		/ 0000 D9F6
	ffree	%st	/ 0010 DDC0
	fincstp		/ 0000 D9F7
	fldcw	0x64(%bp, %si)	/ 0008 D905
	fldenv	0x3e8	/ 0008 D904
	fld1		/ 0000 D9E8
	fldl2t		/ 0000 D9E9
	fldl2e		/ 0000 D9EA
	fldpi		/ 0000 D9EB
	fldlg2		/ 0000 D9EC
	fldln2		/ 0000 D9ED
	fldz		/ 0000 D9EE
	fnclex		/ 0000 DBE2
	fninit		/ 0000 DBE3
	fnop		/ 0000 D9D0
	fpatan		/ 0000 D9F3
	fprem		/ 0000 D9F8
	fprem1		/ 0000 D9F5
	fprem		/ 0000 D9F2
	frndint		/ 0000 D9FC
	frstor	0x3e8	/ 0008 DB04
	fnsave	0x3e8	/ 0008 DB06
	fscale		/ 0000 D9FD
	fsin		/ 0000 D9FE
	fsincos		/ 0000 D9FB
	fsqrt		/ 0000 D9FA
	fnstcw	(%bx, %di)	/ 0008 D907
	fnstenv	0x3e8(%bp)	/ 0008 D906
	ftst		/ 0000 D9E4
	fucomp		/ 0000 DDE9
	fucompp		/ 0000 DAE9
	fwait		/ 0000 009B
	fxam		/ 0000 D9E5
	fxtract		/ 0000 D9F4
	fyl2x		/ 0000 D9F1
	fyl2xp1		/ 0000 D9F9
	hlt		/ 0000 00F4
	icall	0x64(%edi)	/ 000C FF02
	ijmp	0x186a0(%ebp)	/ 000C FF04
	ilcall	(%esi)	/ 000C FF03
	iljmp	(%edi, %esi)	/ 000C FF05
	int	$3	/ 0000 00CC
	int	$0x64	/ 0000 00CD
	into		/ 0002 00CE
	insb		/ 0800 006C
	insw		/ 0802 006D
	insl		/ 0804 006D
	iret		/ 0000 00CF
	iretd		/ 0000 00CF
	ja	.	/ 0000 0007
	jae	abc	/ 0000 0003
	jb	abc	/ 0000 0002
	jbe	abc	/ 0000 0006
	jc	abc	/ 0000 0002
	je	abc	/ 0000 0004
	jz	abc	/ 0000 0004
	jg	.	/ 0000 000F
	jge	abc	/ 0000 000D
	jl	abc	/ 0000 000C
	jle	.	/ 0000 000E
	jna	.	/ 0000 0006
	jnae	.	/ 0000 0002
	jnb	abc	/ 0000 0003
	jnbe	abc	/ 0000 0007
	jnc	.	/ 0000 0003
	jne	abc	/ 0000 0005
	jng	abc	/ 0000 000E
	jnge	.	/ 0000 000C
	jnl	.	/ 0000 000D
	jnle	.	/ 0000 000F
	jno	.	/ 0000 0001
	jnp	abc	/ 0000 000B
	jns	abc	/ 0000 0009
	jnz	abc	/ 0000 0005
	jo	.	/ 0000 0000
	jp	abc	/ 0000 000A
	jpe	abc	/ 0000 000A
	jpo	abc	/ 0000 000B
	js	abc	/ 0000 0008
	jz	abc	/ 0000 0004
	jmp	.	/ 0000 00E9
	jcxz	.	/ 0000 00E3
	jecxz	.	/ 0000 00E3
	lahf		/ 0000 009F
	lcall	$0, $0x186a0	/ 0004 009A
	ljmp	$0, $0x186a0	/ 0004 00EA
	leave		/ 0000 00C9
	lldt	(, %ecx, 2)	/ 0028 0002
	lmsw	0x3e8	/ 0028 0106
	lock		/ 4000 00F0
	lodsb		/ 0800 00AC
	lodsw		/ 0802 00AD
	lodsl		/ 0804 00AD
	loop	.	/ 0000 00E2
	loope	.	/ 0000 00E1
	loopz	.	/ 0000 00E1
	loopne	.	/ 0000 00E0
	loopnz	.	/ 0000 00E0
	ltr	0x64(%bp, %di)	/ 0028 0003
	movsb		/ 0800 00A4
	movsw		/ 0802 00A5
	movsl		/ 0804 00A5
	nop		/ 0000 0090
	outsb		/ 0800 006E
	outsw		/ 0802 006F
	outsl		/ 0804 006F
	rep		/ 1000 00F3
	repe		/ 1000 00F3
	repne		/ 1000 00F2
	ret		/ 0000 00C3
	ret	$0x3e8	/ 0000 00C2
	lret		/ 0000 00CB
	lret	$0x3e8	/ 0000 00CA
	sahf		/ 0000 009E
	scasb		/ 0800 00AE
	scasl		/ 0804 00AF
	scasw		/ 0802 00AF
	seta	%ch	/ 0020 0097
	setae	%ch	/ 0020 0093
	setb	%al	/ 0020 0092
	setbe	0x3e8(%bp)	/ 0020 0096
	setc	(%eax, %ecx, 0)	/ 0020 0092
	sete	0x3e8(%bp, %di)	/ 0020 0094
	setz	0x64(%ebp)	/ 0020 0094
	setg	0x64(%ecx, 4)	/ 0020 009F
	setge	0x3e8	/ 0020 009D
	setl	0x186a0(%eax)	/ 0020 009C
	setle	0x3e8(%esi, %eax, 2)	/ 0020 009E
	setna	(, %edx, 2)	/ 0020 0096
	setnae	%bh	/ 0020 0092
	setnb	0x64(%bx, %di)	/ 0020 0093
	setnbe	0x3e8	/ 0020 0097
	setnc	0x3e8	/ 0020 0093
	setne	0x3e8	/ 0020 0095
	setng	%dh	/ 0020 009E
	setnge	0x3e8(%si)	/ 0020 009C
	setnl	0x3e8	/ 0020 009D
	setnle	%bl	/ 0020 009F
	setno	%bl	/ 0020 0091
	setnp	0x64(%bp)	/ 0020 009B
	setns	0x186a0(%eax)	/ 0020 0099
	setnz	(, %ebp, 8)	/ 0020 0095
	seto	(%eax, 2)	/ 0020 0090
	setp	(%ebx, %esi)	/ 0020 009A
	setpe	0x3e8(%bx, %si)	/ 0020 009A
	setpo	(%bp, %si)	/ 0020 009B
	sets	0x3e8	/ 0020 0098
	setz	0x64(%bx, %di)	/ 0020 0094
	sgdt	abc	/ 0028 0100
	sidt	abc	/ 0028 0101
	sldt	0x3e8	/ 0028 0000
	smsw	%si	/ 0028 0104
	stc		/ 0000 00F9
	std		/ 0000 00FD
	sti		/ 0000 00FB
	stosb		/ 0800 00AA
	stosw		/ 0802 00AB
	stosl		/ 0804 00AB
	str		/ 0028 0001
	verr	%di	/ 0028 0004
	verw	%si	/ 0028 0005
	wait		/ 0000 009B
	xlat		/ 0000 00D7
	xlatb		/ 0000 00D7
	adcl	$064, %ecx	/ 200C 8302
	adcw	$-5, 0x4(%ecx, %ebp)	/ 200A 8302
	adcb	$0x64, %al	/ 0000 0014
	adcw	$0x3e8, %ax	/ 0002 0015
	adcl	$0x186a0, %eax	/ 0004 0015
	adcl	$0x186a0	/ 0004 0015
	adcb	$0x64, 0x64(%bx, %si)	/ 2008 8002
	adcw	$0x3e8, 0x3e8	/ 200A 8102
	adcl	$0x186a0, (, %edi, 1)	/ 200C 8102
	adcb	%bl, %ch	/ 2000 0012
	adcw	0x64(%di), %si	/ 2002 0013
	adcl	(%edx, %ebp, 0), %esi	/ 2004 0013
	adcb	%ah, 0x3e8	/ 2000 0010
	adcw	%dx, 0x64(%di)	/ 2002 0011
	adcl	%ecx, 0x64(%ebp)	/ 2004 0011
	addl	$-5, 0x64(%bx)	/ 200C 8300
	addw	$064, 0x3e8(%bx)	/ 200A 8300
	addb	$0x64, %al	/ 2000 0004
	addw	$0x3e8, %ax	/ 2002 0005
	addl	$0x186a0, %eax	/ 2004 0005
	addl	$0x186a0	/ 2004 0005
	addb	$0x64, %bl	/ 2008 8000
	addw	$0x3e8, 0x3e8	/ 200A 8100
	addl	$0x186a0, %esp	/ 200C 8100
	addb	(%bp, %si), %cl	/ 2000 0002
	addw	(%edi, %ebp), %bp	/ 2002 0003
	addl	0x64(%bx), %edi	/ 2004 0003
	addb	%dh, %dl	/ 2000 0000
	addw	%cx, 0x64(%bp, %di)	/ 2002 0001
	addl	%ebx, 0x3e8	/ 2004 0001
	andl	$-5, %esi	/ 200C 8304
	andw	$064, (%ebp, 8)	/ 200A 8304
	andb	$0x64, %al	/ 2000 0024
	andw	$0x3e8, %ax	/ 2002 0025
	andl	$0x186a0, %eax	/ 2004 0025
	andl	$0x186a0	/ 2004 0025
	andb	$0x64, 0x3e8	/ 2008 8004
	andw	$0x3e8, 0x3e8	/ 200A 8104
	andl	$0x186a0, %ecx	/ 200C 8104
	andb	0x3e8, %dh	/ 2000 0022
	andw	(%bx, %si), %di	/ 2002 0023
	andl	0x3e8, %eax	/ 2004 0023
	andb	%dl, %bh	/ 2000 0020
	andw	%cx, 0x64(%di)	/ 2002 0021
	andl	%edx, (%eax, %esi)	/ 2004 0021
	boundw	0x3e8, %cx	/ 0002 0062
	boundl	0x3e8, %eax	/ 0004 0062
	bsfw	0x3e8, %ax	/ 0022 00BC
	bsfl	0x3e8(%bp, %si), %eax	/ 0024 00BC
	bsrw	(%ebp, %edi), %si	/ 0022 00BD
	bsrl	0x3e8, %ecx	/ 0024 00BD
	btw	%si, 0x3e8(%bx)	/ 0022 00A3
	btl	%esp, 0x3e8	/ 0024 00A3
	btw	$0x64, (, %edx, 8)	/ 002A BA04
	btl	$0x64, %edx	/ 002C BA04
	btcw	%ax, %dx	/ 2022 00BB
	btcl	%ebx, (%ecx, 0)	/ 2024 00BB
	btcw	$0x64, %dx	/ 202A BA07
	btcl	$0x64, 0x3e8(%edx, %ecx, 1)	/ 202C BA07
	btrw	%ax, 0x3e8	/ 2022 00B3
	btrl	%ebx, (%bp, %si)	/ 2024 00B3
	btrw	$0x64, 0x3e8	/ 202A BA06
	btrl	$0x64, 0x3e8(%bx, %si)	/ 202C BA06
	btsw	%cx, 0x64(%di)	/ 2022 00AB
	btsl	%edx, 0x3e8	/ 2024 00AB
	btsw	$0x64, 0x64(%ecx, 0)	/ 202A BA05
	btsl	$0x64, (%bp, %si)	/ 202C BA05
	cmpl	$-5, 0x3e8	/ 000C 8307
	cmpw	$064, %bx	/ 000A 8307
	cmpb	$0x64, %al	/ 0000 003C
	cmpw	$0x3e8, %ax	/ 0002 003D
	cmpl	$0x186a0, %eax	/ 0004 003D
	cmpl	$0x186a0	/ 0004 003D
	cmpb	$0x64, 0x3e8	/ 0008 8007
	cmpw	$0x3e8, 0x3e8(%bp)	/ 000A 8107
	cmpl	$0x186a0, 0x3e8	/ 000C 8107
	cmpb	0x3e8(%bx), %bl	/ 0000 003A
	cmpw	0x64(%bx), %bp	/ 0002 003B
	cmpl	0x64(%eax), %ebp	/ 0004 003B
	cmpb	%ah, 0x3e8	/ 0000 0038
	cmpw	%dx, 0x3e8	/ 0002 0039
	cmpl	%ebp, 0x3e8	/ 0004 0039
	decw	%sp	/ 0012 0048
	decl	%edi	/ 0014 0048
	decb	0x3e8(%bx, %di)	/ 2008 FE01
	decw	0x64(%bp, %di)	/ 200A FF01
	decl	0x3e8	/ 200C FF01
	divb	%dl, %al	/ 0008 F606
	divb	0x3e8	/ 0008 F606
	divw	(%ecx, %ecx, 0), %ax	/ 000A F706
	divw	0x3e8	/ 000A F706
	divl	(%esi, %ecx, 1), %eax	/ 000C F706
	divl	0x3e8	/ 000C F706
	fadds	0x3e8	/ 0008 D800
	faddl	(%si)	/ 0008 DC00
	fadd	%st(7), %st	/ 0010 D8C0
	fadd	%st(6)	/ 0010 D8C0
	fadd		/ 0000 D8C1
	fadd	%st, %st(6)	/ 0010 DCC0
	faddp	%st, %st(0)	/ 0010 DEC0
	faddp	%st(5)	/ 0010 DEC0
	faddp		/ 0000 DEC1
	fcoms	(%eax)	/ 0008 D802
	fcoml	0x3e8(%bp, %si)	/ 0008 DC02
	fcom	%st(7), %st	/ 0010 D8D0
	fcom	%st(6)	/ 0010 D8D0
	fcom		/ 0000 D8D1
	fcomps	0x3e8(%bx, %di)	/ 0008 D803
	fcompl	0x3e8	/ 0008 DC03
	fcomp	%st(7)	/ 0010 D8D8
	fcomp		/ 0000 D8D9
	fdivs	(%di)	/ 0008 D806
	fdivl	(%si)	/ 0008 DC06
	fdiv	%st, %st	/ 0010 D8F0
	fdiv	%st(3)	/ 0010 D8F0
	fdiv		/ 0000 D8F1
	fdiv	%st, %st(3)	/ 0010 DCF0
	fdivp	%st, %st	/ 0010 DEF0
	fdivp	%st(0)	/ 0010 DEF0
	fdivp		/ 0000 DEF1
	fdivrs	0x3e8(%edx, %edx, 0)	/ 0008 D807
	fdivrl	0x64(%bp)	/ 0008 DC07
	fdivr	%st(1), %st	/ 0010 D8F8
	fdivr	%st(4)	/ 0010 D8F8
	fdivr		/ 0000 D8F9
	fdivr	%st, %st(4)	/ 0010 DCF8
	fdivrp	%st, %st(5)	/ 0010 DEF8
	fdivrp	%st(7)	/ 0010 DEF8
	fdivrp		/ 0000 DEF9
	fiaddl	0x3e8	/ 0008 DA00
	fiadds	0x3e8(%bp, %di)	/ 0008 DE00
	ficoml	0x3e8(%bp, %si)	/ 0008 DA02
	ficoms	0x64(%ebp, 2)	/ 0008 DE02
	ficompl	0x3e8	/ 0008 DA03
	ficomps	0x3e8	/ 0008 DE03
	fidivl	(%bx, %di)	/ 0008 DA06
	fidivs	0x3e8(%bx, %di)	/ 0008 DE06
	fidivrl	(%di)	/ 0008 DA07
	fidivrs	0x64(%ecx)	/ 0008 DE07
	fildl	0x3e8	/ 0008 DB00
	filds	0x186a0(%eax)	/ 0008 DF00
	fildll	0x3e8(%bp)	/ 0008 DF05
	fimull	(%bp, %si)	/ 0008 DA01
	fimuls	(%edx)	/ 0008 DE01
	fistl	0x3e8	/ 0008 DB02
	fists	0x3e8	/ 0008 DF02
	fistpl	0x3e8	/ 0008 DB03
	fistps	0x64(%bx)	/ 0008 DF03
	fistpll	(%esi, %ecx)	/ 0008 DF07
	fisubl	0x3e8(%bx, %si)	/ 0008 DA04
	fisubs	(%esi)	/ 0008 DE04
	fisubrl	0x3e8	/ 0008 DA05
	fisubrs	0x3e8(%ebp, %ecx, 4)	/ 0008 DE05
	fld	%st(5)	/ 0010 D9C0
	flds	0x3e8	/ 0008 D900
	fldl	0x4(%eax, %edx)	/ 0008 DD00
	fldt	0x64(%di)	/ 0008 DB05
	fmuls	(%edx)	/ 0008 D801
	fmull	0x3e8(%bx, %di)	/ 0008 DC01
	fmul	%st(3), %st	/ 0010 D8C8
	fmul	%st(6)	/ 0010 D8C8
	fmul		/ 0000 D8C9
	fmul	%st, %st(4)	/ 0010 DCC8
	fmulp	%st, %st(5)	/ 0010 DEC8
	fmulp	%st(6)	/ 0010 DEC8
	fmulp		/ 0000 DEC9
	fst	%st(4)	/ 0010 DDD0
	fsts	(%bx, %di)	/ 0008 D902
	fstl	0x4(%eax, %edi)	/ 0008 DD02
	fstp	%st(5)	/ 0010 DDD8
	fstps	(%bp, %si)	/ 0008 D903
	fstpl	0x186a0(%ebp)	/ 0008 DD03
	fstpt	0x3e8(%bp, %si)	/ 0008 DB07
	fnstsw	0x64(%bx, %di)	/ 0008 DD07
	fnstsw	%ax	/ 0000 DFE0
	fsubs	(%edx, %esi, 4)	/ 0008 D804
	fsubl	0x3e8	/ 0008 DC04
	fsub	%st(1), %st	/ 0010 D8E0
	fsub	%st(3)	/ 0010 D8E0
	fsub		/ 0000 D8E1
	fsub	%st, %st(1)	/ 0010 DCE0
	fsubp	%st, %st(3)	/ 0010 DEE0
	fsubp	%st	/ 0010 DEE0
	fsubp		/ 0000 DEE1
	fsubrs	(%edi, %edx, 2)	/ 0008 D805
	fsubrl	0x3e8	/ 0008 DC05
	fsubr	%st(7), %st	/ 0010 D8E8
	fsubr	%st(6)	/ 0010 D8E8
	fsubr		/ 0000 D8E9
	fsubr	%st, %st(1)	/ 0010 DCE8
	fsubrp	%st, %st(4)	/ 0010 DEE8
	fsubrp	%st(2)	/ 0010 DEE8
	fsubrp		/ 0000 DEE9
	fucom	%st, %st(1)	/ 0010 DDE0
	fucom	%st	/ 0010 DDE0
	fucom		/ 0000 DDE1
	fxch	%st, %st(5)	/ 0010 D9C8
	fxch	%st(7), %st	/ 0010 D9C8
	fxch	%st(0)	/ 0010 D9C8
	fxch		/ 0000 D9C9
	idivb	0x3e8, %al	/ 0008 F607
	idivb	0x64(%bx, %si)	/ 0008 F607
	idivw	%dx, %ax	/ 000A F707
	idivw	0x186a0(%edi)	/ 000A F707
	idivl	(%ebp, 0), %eax	/ 000C F707
	idivl	(%bx, %si)	/ 000C F707
	imulb	0x4(%ebx, %esi), %al	/ 0008 F605
	imulb	0x3e8(%bx, %di)	/ 0008 F605
	imulw	0x3e8(%ebp, %edi, 4), %ax	/ 000A F705
	imulw	0x3e8	/ 000A F705
	imull	(%bx), %eax	/ 000C F705
	imull	0x3e8	/ 000C F705
	imulw	(%bx), %di	/ 0022 00AF
	imull	0x3e8, %edi	/ 0024 00AF
	imulw	$-5, 0x64(%bp, %di), %ax	/ 0002 006B
	imull	$064, 0x64(%bp, %si), %ebx	/ 0004 006B
	imulw	$064, %si	/ 0202 006B
	imull	$-5, %esi	/ 0204 006B
	imulw	$0x3e8, 0x3e8, %bp	/ 0002 0069
	imull	$0x186a0, 0x3e8, %eax	/ 0004 0069
	imulw	$0x3e8, %di	/ 0202 0069
	imull	$0x186a0, %esi	/ 0204 0069
	inb	$0x64	/ 0000 00E4
	inw	$0x64	/ 0002 00E5
	inl	$0x64	/ 0004 00E5
	inb	(%dx)	/ 0000 00EC
	inw	(%dx)	/ 0002 00ED
	inl	(%dx)	/ 0004 00ED
	incw	%bx	/ 0012 0040
	incl	%ebp	/ 0014 0040
	incb	0x3e8	/ 2008 FE00
	incw	%cx	/ 200A FF00
	incl	%ebx	/ 200C FF00
	larw	(%si), %sp	/ 0022 0002
	larl	0x64(%bx, %si), %eax	/ 0024 0002
	leaw	(%bp, %di), %ax	/ 0002 008D
	leal	0x64(%bx, %si), %esi	/ 0004 008D
	lgdtw	0x3e8	/ 002A 0102
	lgdtl	0x3e8	/ 002C 0102
	lgdtw	(, %edi, 2)	/ 002A 0103
	lgdtl	0x3e8	/ 002C 0103
	ldsw	(%edx, %ebp, 0), %dx	/ 0002 00C5
	ldsl	0x4(%eax, %edx), %ecx	/ 0004 00C5
	lssw	0x3e8, %bp	/ 0022 00B2
	lssl	0x3e8, %ebx	/ 0024 00B2
	lesw	(%bx, %si), %si	/ 0002 00C4
	lesl	0x64(%edx, 1), %eax	/ 0004 00C4
	lfsw	(%di), %bx	/ 0022 00B4
	lfsl	0x186a0(%ebp), %edi	/ 0024 00B4
	lgsw	0x64(%bp, %di), %bx	/ 0022 00B5
	lgsl	0x64(%ebp, 4), %edi	/ 0024 00B5
	lslw	%si, %di	/ 0022 0003
	lsll	0x3e8(%si), %esp	/ 0024 0003
	movb	abc, %al	/ 0000 00A0
	movw	abc, %ax	/ 0002 00A1
	movl	abc, %eax	/ 0004 00A1
	movb	%al, abc	/ 0000 00A2
	movw	%ax, abc	/ 0002 00A3
	movl	%eax, abc	/ 0004 00A3
	movb	0x64(%edx, 8), %dh	/ 0000 008A
	movw	0x3e8(%bx), %bx	/ 0002 008B
	movl	(%bx), %ebx	/ 0004 008B
	movb	%al, %dl	/ 0000 0088
	movw	%bx, (%si)	/ 0002 0089
	movl	%ebp, 0x3e8(%bp, %di)	/ 0004 0089
	movw	%ss, 0x186a0(%esi)	/ 0000 008C
	movw	0x3e8, %fs	/ 0000 008E
	movb	$0x64, %bh	/ 0010 00B0
	movw	$0x3e8, %bp	/ 0012 00B8
	movl	$0x186a0, %esp	/ 0014 00B8
	movb	$0x64, 0x64(%edi)	/ 0000 00C6
	movw	$0x3e8, 0x3e8	/ 0002 00C7
	movl	$0x186a0, 0x3e8	/ 0004 00C7
	movl	%cr3, %edx	/ 0024 0020
	movl	%ebp, %cr0	/ 0024 0022
	movl	%dr2, %ebp	/ 0024 0021
	movl	%ecx, %dr3	/ 0024 0023
	movl	%tr7, %ebx	/ 0024 0024
	movl	%edi, %tr7	/ 0024 0026
	movsxb	0x3e8, %bp	/ 0022 00BE
	movsxb	%ch, %ecx	/ 0024 00BE
	movsxw	0x64(%si), %ebp	/ 8024 00BF
	movsbw	(%bp, %si), %sp	/ 0022 00BE
	movsbl	0x186a0(%eax), %ecx	/ 0024 00BE
	movswl	(%bx, %si), %ecx	/ 8024 00BF
	movzxb	0x3e8, %bx	/ 0022 00B6
	movzxb	0x64(%di), %edx	/ 0024 00B6
	movzxw	0x4(%ebx, %ebp), %esp	/ 8024 00B7
	movzbw	%dh, %ax	/ 0022 00B6
	movzbl	0x64(%bp, %si), %eax	/ 0024 00B6
	movzwl	0x64(%bp, %di), %ebp	/ 8024 00B7
	mulb	(, %eax, 2), %al	/ 0008 F604
	mulb	0x3e8	/ 0008 F604
	mulw	(%ecx, 2), %ax	/ 000A F704
	mulw	0x3e8	/ 000A F704
	mull	0x3e8(%bx, %si), %eax	/ 000C F704
	mull	0x3e8(%si)	/ 000C F704
	negb	0x3e8	/ 2008 F603
	negw	(%bp, %si)	/ 200A F703
	negl	(%ebx, %edi, 1)	/ 200C F703
	notb	(%edi)	/ 2008 F602
	notw	%bp	/ 200A F702
	notl	0x64(%eax)	/ 200C F702
	orl	$064, 0x4(%edx, %esi)	/ 200C 8301
	orw	$064, 0x64(%esi, 8)	/ 200A 8301
	orb	$0x64, %al	/ 2000 000C
	orw	$0x3e8, %ax	/ 2002 000D
	orl	$0x186a0, %eax	/ 2004 000D
	orl	$0x186a0	/ 2004 000D
	orb	$0x64, 0x186a0(%edx)	/ 2008 8001
	orw	$0x3e8, (%ebp, %eax)	/ 200A 8101
	orl	$0x186a0, 0x3e8	/ 200C 8101
	orb	0x3e8, %bl	/ 2000 000A
	orw	%si, %di	/ 2002 000B
	orl	0x64(%di), %ebx	/ 2004 000B
	orb	%bh, (%bx, %si)	/ 2000 0008
	orw	%di, 0x3e8	/ 2002 0009
	orl	%edx, 0x3e8(%di)	/ 2004 0009
	outb	$0x64	/ 0000 00E6
	outw	$0x64	/ 0002 00E7
	outl	$0x64	/ 0004 00E7
	outb	(%dx)	/ 0000 00EE
	outw	(%dx)	/ 0002 00EF
	outl	(%dx)	/ 0004 00EF
	popw	%bp	/ 0012 0058
	popl	%edx	/ 0014 0058
	popw	%ds	/ 0000 001F
	popw	%es	/ 0000 0007
	popw	%ss	/ 0000 0017
	popw	%fs	/ 0020 00A1
	popw	%gs	/ 0020 00A9
	popw	0x3e8	/ 000A 8F00
	popl	(, %eax, 8)	/ 000C 8F00
	popaw		/ 0002 0061
	popal		/ 0004 0061
	popfw		/ 0002 009D
	popfl		/ 0004 009D
	pushw	%bp	/ 0012 0050
	pushl	%eax	/ 0014 0050
	pushb	$-5	/ 0000 006A
	pushw	$0x3e8	/ 0002 0068
	pushl	$0x186a0	/ 0004 0068
	pushw	%cs	/ 0000 000E
	pushw	%ds	/ 0000 001E
	pushw	%es	/ 0000 0006
	pushw	%ss	/ 0000 0016
	pushw	%fs	/ 0020 00A0
	pushw	%gs	/ 0020 00A8
	pushw	(, %edx, 8)	/ 000A FF06
	pushl	0x64(%esi)	/ 000C FF06
	pushaw		/ 0002 0060
	pushal		/ 0004 0060
	pushfw		/ 0002 009C
	pushfl		/ 0004 009C
	rclb	$1, 0x64(%si)	/ 0008 D002
	rclb	%cl, 0x3e8(%bx)	/ 0008 D202
	rclb	$0x64, %bl	/ 0008 C002
	rclw	$1, (%edi, %eax)	/ 000A D102
	rclw	%cl, (%ebp, %ebx, 0)	/ 000A D302
	rclw	$0x64, 0x64(%si)	/ 000A C102
	rcll	$1, 0x3e8(%bx, %di)	/ 000C D102
	rcll	%cl, 0x3e8	/ 000C D302
	rcll	$0x64, (%bx, %si)	/ 000C C102
	rcrb	$1, (, %ebp, 8)	/ 0008 D003
	rcrb	%cl, (%ebp, 0)	/ 0008 D203
	rcrb	$0x64, 0x3e8	/ 0008 C003
	rcrw	$1, (%bx, %si)	/ 000A D103
	rcrw	%cl, (%bx)	/ 000A D303
	rcrw	$0x64, (%bx)	/ 000A C103
	rcrl	$1, 0x64(%di)	/ 000C D103
	rcrl	%cl, 0x64(%si)	/ 000C D303
	rcrl	$0x64, (%edi, 2)	/ 000C C103
	rolb	$1, %ch	/ 0008 D000
	rolb	%cl, 0x64(%ebx)	/ 0008 D200
	rolb	$0x64, 0x3e8(%bp)	/ 0008 C000
	rolw	$1, %sp	/ 000A D100
	rolw	%cl, 0x3e8(%bx)	/ 000A D300
	rolw	$0x64, 0x64(%bx, %si)	/ 000A C100
	roll	$1, (%ebp, 8)	/ 000C D100
	roll	%cl, 0x64(%di)	/ 000C D300
	roll	$0x64, %edx	/ 000C C100
	rorb	$1, 0x3e8	/ 0008 D001
	rorb	%cl, (%edx, %esi, 0)	/ 0008 D201
	rorb	$0x64, (, %eax, 1)	/ 0008 C001
	rorw	$1, 0x3e8(%di)	/ 000A D101
	rorw	%cl, 0x64(%di)	/ 000A D301
	rorl	$1, (%bp, %di)	/ 000C D101
	rorl	%cl, 0x64(%bx, %si)	/ 000C D301
	salb	$1, 0x3e8	/ 0008 D004
	salb	%cl, %bh	/ 0008 D204
	salb	$0x64, 0x64(%si)	/ 0008 C004
	salw	$1, %cx	/ 000A D104
	salw	%cl, (%bp, %si)	/ 000A D304
	salw	$0x64, 0x64(%bx, %di)	/ 000A C104
	sall	$1, 0x64(%bp, %si)	/ 000C D104
	sall	%cl, (%bx, %si)	/ 000C D304
	sall	$0x64, 0x3e8(%bx)	/ 000C C104
	shlb	$1, 0x64(%di)	/ 0008 D004
	shlb	%cl, %cl	/ 0008 D204
	shlb	$0x64, 0x3e8	/ 0008 C004
	shlw	$1, (%edx)	/ 000A D104
	shlw	%cl, (%bp, %di)	/ 000A D304
	shlw	$0x64, 0x64(%bx)	/ 000A C104
	shll	$1, 0x3e8	/ 000C D104
	shll	%cl, 0x186a0(%ebp)	/ 000C D304
	shll	$0x64, 0x3e8(%bx, %si)	/ 000C C104
	sarb	$1, %ah	/ 0008 D007
	sarb	%cl, (%esi, 2)	/ 0008 D207
	sarb	$0x64, 0x64(%ebx)	/ 0008 C007
	sarw	$1, 0x3e8(%bx, %di)	/ 000A D107
	sarw	%cl, 0x3e8	/ 000A D307
	sarw	$0x64, 0x3e8(%edx, %ebp, 4)	/ 000A C107
	sarl	$1, (%ebx, 4)	/ 000C D107
	sarl	%cl, 0x3e8	/ 000C D307
	sarl	$0x64, (%edx)	/ 000C C107
	shrb	$1, 0x3e8	/ 0008 D005
	shrb	%cl, 0x4(%edi, %eax)	/ 0008 D205
	shrb	$0x64, 0x3e8	/ 0008 C005
	shrw	$1, 0x3e8	/ 000A D105
	shrw	%cl, %sp	/ 000A D305
	shrw	$0x64, (%ecx)	/ 000A C105
	shrl	$1, %edx	/ 000C D105
	shrl	%cl, %edx	/ 000C D305
	shrl	$0x64, (%edx, 4)	/ 000C C105
	sbbl	$064, (%bp, %di)	/ 200C 8303
	sbbw	$-5, %bp	/ 200A 8303
	sbbb	$0x64, %al	/ 2000 001C
	sbbw	$0x3e8, %ax	/ 2002 001D
	sbbl	$0x186a0, %eax	/ 2004 001D
	sbbl	$0x186a0	/ 2004 001D
	sbbb	$0x64, 0x3e8	/ 2008 8003
	sbbw	$0x3e8, (%bp, %si)	/ 200A 8103
	sbbl	$0x186a0, 0x3e8	/ 200C 8103
	sbbb	0x3e8, %ah	/ 2000 001A
	sbbw	0x3e8(%bx, %di), %bp	/ 2002 001B
	sbbl	%ecx, %edi	/ 2004 001B
	sbbb	%cl, 0x3e8	/ 2000 0018
	sbbw	%ax, %si	/ 2002 0019
	sbbl	%ebp, (%bp, %di)	/ 2004 0019
	shldw	$0x64, %bx, %di	/ 0022 00A4
	shldl	$0x64, %ebp, 0x3e8	/ 0024 00A4
	shldw	%cl, %cx, %bp	/ 0022 00A5
	shldl	%cl, %eax, %edx	/ 0024 00A5
	shrdw	$0x64, %sp, %bp	/ 0022 00AC
	shrdl	$0x64, %ecx, (%eax)	/ 0024 00AC
	shrdw	%cl, %bp, (%ebp, %ecx)	/ 0022 00AD
	shrdl	%cl, %edx, 0x3e8(%edi, %esi, 0)	/ 0024 00AD
	shrdw	%ax, 0x3e8(%bx, %si)	/ 0022 00AD
	shrdl	%edx, 0x3e8	/ 0024 00AD
	subl	$064, 0x3e8	/ 200C 8305
	subw	$064, 0x3e8	/ 200A 8305
	subb	$0x64, %al	/ 2000 002C
	subw	$0x3e8, %ax	/ 2002 002D
	subl	$0x186a0, %eax	/ 2004 002D
	subl	$0x186a0	/ 2004 002D
	subb	$0x64, 0x3e8	/ 2008 8005
	subw	$0x3e8, 0x3e8(%si)	/ 200A 8105
	subl	$0x186a0, 0x3e8	/ 200C 8105
	subb	0x64(%eax, 4), %al	/ 2000 002A
	subw	0x3e8(%ebx, %edi, 2), %bp	/ 2002 002B
	subl	0x64(%esi), %eax	/ 2004 002B
	subb	%al, 0x3e8	/ 2000 0028
	subw	%sp, %di	/ 2002 0029
	subl	%esi, %eax	/ 2004 0029
	testb	$0x64, %al	/ 0000 00A8
	testw	$0x3e8, %ax	/ 0002 00A9
	testl	$0x186a0, %eax	/ 0004 00A9
	testl	$0x186a0	/ 0004 00A9
	testb	$0x64, (%ebp, %ebx)	/ 0008 F600
	testw	$0x3e8, 0x3e8	/ 000A F700
	testl	$0x186a0, 0x3e8	/ 000C F700
	testb	%cl, 0x3e8(%di)	/ 0000 0084
	testw	%bp, %bx	/ 0002 0085
	testl	%eax, 0x3e8	/ 0004 0085
	xchgw	%dx, %ax	/ 0012 0090
	xchgw	%ax, %sp	/ 0012 0090
	xchgl	%ebx, %eax	/ 0014 0090
	xchgl	%edx	/ 0014 0090
	xchgl	%eax, %edi	/ 0014 0090
	xchgb	0x4(%ecx, %edi), %dl	/ 2000 0086
	xchgw	0x3e8, %cx	/ 2002 0087
	xchgl	0x3e8, %ebx	/ 2004 0087
	xchgb	%dh, 0x3e8	/ 2000 0086
	xchgw	%dx, %di	/ 2002 0087
	xchgl	%edi, 0x3e8	/ 2004 0087
	xorl	$064, 0x3e8	/ 200C 8306
	xorw	$064, 0x3e8	/ 200A 8306
	xorb	$0x64, %al	/ 2000 0034
	xorw	$0x3e8, %ax	/ 2002 0035
	xorl	$0x186a0, %eax	/ 2004 0035
	xorl	$0x186a0	/ 2004 0035
	xorb	$0x64, 0x3e8	/ 2008 8006
	xorw	$0x3e8, %ax	/ 200A 8106
	xorl	$0x186a0, %esp	/ 200C 8106
	xorb	(%edi, %ebp, 0), %dl	/ 2000 0032
	xorw	0x3e8, %bp	/ 2002 0033
	xorl	0x64(%esi), %ebp	/ 2004 0033
	xorb	%dl, (, %ecx, 0)	/ 2000 0030
	xorw	%bp, 0x3e8	/ 2002 0031
	xorl	%ebx, 0x3e8(%bp)	/ 2004 0031
