	.llen	100
	.globl	_start, environ, __fltused, errno, main, exit
	.lcomm	environ, 4
	.lcomm	__fltused, 4
	.lcomm	errno, 4

_start:	subl	$8, %esp
	movl	%esp, %ebp
	movl	8(%ebp), %eax			/ get argc
	leal	16(%ebp, %eax, 4), %edx		/ get environment
	movl	%edx, environ			/ save environment
	pushl	%edx				/ push environment
	leal	12(%ebp), %edx			/ get argv
	pushl	%edx				/ push argv
	pushl	%eax				/ push argc
	call	main
	addl	$12, %esp			/ pop stack
	pushl	%eax				/ push main's rv
	call	exit
