#
# hellola.s: Linux i386 hello-world program in GNU as(1) syntax >=2.9.5
# by pts@fazekas.hy at Sun Apr  7 20:03:18 CEST 2024
#
# Tested with GNU as(1) 2.9.5, 2.22, 2.23.52 (i686-w64-mingw32), 2.30 and
# later versions.
#
# Compile with: as --32 -march=i386 -o hellola.o hellola.s
# Compile with earlier as(1) versions: as -o hellola.o hellola.s
#

do_msg2:
	mov $4, %eax  # SYS_write.
	mov $1, %ebx  # STDOUT_FILENO.
	mov msgdp, %ecx
	mov $msgd_end1-1-msgd, %edx
	int $0x80  # Linux i386 syscall.
do_exit:
	mov $1, %eax  # SYS_exit.
	xor %ebx, %ebx  # EXIT_SUCCESS == 0.
	dec %ebx
	add bssvar, %bl  # Adds 1, because Linux initializes bssvar to 0, and we've incremented it below. EBX becomes -256, practically exit(0).
	int $0x80  # Linux i386 syscall.
.globl _start
_start:  # Entry point by GNU ld(1).
	mov $4, %eax  # SYS_write.
	mov $1, %ebx  # STDOUT_FILENO.
	mov $msg, %ecx
	mov $msg_end1-1-msg, %edx
	int $0x80  # Linux i386 syscall.

	mov $4, %eax  # SYS_write.
	mov $1, %ebx  # STDOUT_FILENO.
	mov msgrp, %ecx
	mov $msgr_end1-1-msgr, %edx
	int $0x80  # Linux i386 syscall.

	movb $'r', msgd+2
	incb bssvar  # This is still wrong. Each time 4 bytes are added to the size of .text, .bss moves not only by 4 bytes, but by 0xc bytes.
	jmp do_msg2
	incb bssvar  # Not reached.
	jmp do_exit  # Not reached.
	hlt  # Not reached.

msg:	.string "Hel"
msg_end1:
label123456789:  # Long label.

.data
msgd:	.string "WoXld!\n"  # The letter X will be changed to r above.
msgd_end1:
msgrp:	.long msgr
msgdp:	.long msgd

.text


msgr:	.string "lo, "  # This also adds a NUL ("\0"), but there is no .ascii directive in some assemblers.
msgr_end1:

.bss
.fill 6, 1, 0
bssvar:
.fill 0x130-5  # Also: .zero 0x130-5
