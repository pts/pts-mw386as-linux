/
/ hellol.s: Linux i386 hello-world program in the Mark Williams 80386 assembler syntax
/ by pts@fazekas.hy at Sun Apr  7 20:03:18 CEST 2024
/

do_msg2:
	mov $4, %eax  / SYS_write.
	mov $1, %ebx  / STDOUT_FILENO.
	mov $msgd, %ecx
	mov $msgd_end1-1-msgd, %edx
	int $0x80  / Linux i386 syscall.
do_exit:
	mov $1, %eax  / SYS_exit.
	xor %ebx, %ebx  / EXIT_SUCCESS == 0.
	dec %ebx
	add bssvar, %bl  / Adds 1, because Linux initializes bssvar to 0, and we've incremented it below.
	int $0x80  / Linux i386 syscall.
.globl _start
_start:  / Entry point by GNU ld(1).
	mov $4, %eax  / SYS_write.
	mov $1, %ebx  / STDOUT_FILENO.
	mov $msg, %ecx
	mov $msg_end1-1-msg, %edx
	int $0x80  / Linux i386 syscall.

	movb $'r', msgd+2
	incb bssvar  / This is still wrong. Each time 4 bytes are added to the size of .text, .bss moves not only by 4 bytes, but by 0xc bytes.
	jmp do_msg2
	incb bssvar  / Not reached.
	jmp do_exit  / Not reached.
	hlt  / Not reached.

msg:	.string "Hello, "
msg_end1:
label123456789:  / Long label.

.data
msgd:	.string "WoXld!\n"  / The letter X will be changed to r above.
msgd_end1:

.bss
.blkb 6  / Moves correctly, 1 byte at a time.
bssvar:
.blkb 0x130-5  / GNU as(1) has this (also `.fill ...`): .zero 0x130

/ There is also `.section .rodata' supported by the assembler, but
/ fixcoff.pl doesn't support it. Also the assembler puts .rodata after .bss.
