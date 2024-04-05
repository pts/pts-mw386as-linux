.globl _start

_start:
mov $4, %eax  / SYS_write.
mov $1, %ebx  / STDOUT_FILENO.
mov $msg, %ecx
mov $msg_end1-1-msg, %edx
int $0x80  / Linux i386 syscall.

mov $1, %eax  / SYS_exit.
xor %ebx, %ebx  / EXIT_SuCCESS == 0.
int $0x80  / Linux i386 syscall.

msg: .string "Hello, World!\n"
msg_end1:
