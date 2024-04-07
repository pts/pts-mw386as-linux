; by pts@fazekas.hu at Fri Apr  5 03:00:43 CEST 2024

; Idx Name          Size      VMA       File off  Algn
;   0 .text         0x00ed3c  0x0000a8  0x0000a8  2**2  CONTENTS, ALLOC, LOAD, READONLY, CODE, NOREAD
;   1 .data         0x001fbc  0x400de4  0x00ede4  2**2  CONTENTS, ALLOC, LOAD, READONLY, DATA, NOREAD
;   2 .bss          0x402da0  0x402da0  0x000000  2**2  ALLOC, READONLY, NOREAD

bits 32
cpu 386
program_base equ 0
data_base equ 0x400000-0xe000
org program_base

OSABI_Linux equ 3
Elf32_Ehdr:
		db 0x7F,'ELF',1,1,1,OSABI_Linux,0,0,0,0,0,0,0,0,2,0,3,0
		dd 1,_start,Elf32_Phdr0-Elf32_Ehdr,0,0
		dw Elf32_Phdr0-Elf32_Ehdr,0x20,(Elf32_Phdr_end-Elf32_Phdr0)/0x20,0,0,0
; LOAD off 0x00000000 vaddr 0x00000000 paddr 0x00000000 align 2**12 filesz 0x0000ede4 memsz 0x0000ede4 flags r-x
Elf32_Phdr0:	dd 1,0,program_base,program_base,text_end-program_base,text_end-program_base,5,0x1000
; LOAD off 0x0000ede4 vaddr 0x00400de4 paddr 0x00400de4 align 2**12 filesz 0x00001fbc memsz 0x00002a68 flags rw-
Elf32_Phdr1:	dd 1,text_end,data_base+text_end,data_base+text_end,data_end-data,data_end-data+bss_size,6,0x1000
Elf32_Phdr_end:

;section .text align=1
%macro copy_to 1
  %if (%1)-($-$$)<0
    times (%1)-($-$$) nop
  %else
    incbin 'as', ($-$$), (%1)-($-$$)
  %endif
%endm
%macro fill_to 2
  %ifidn %2, copy
    copy_to %1
  %else
    times (%1)-($-$$) %2
  %endif
%endm
errno equ 0x403824  ; In .bss.
brk_end_ptr equ 0x402c04  ; uint32_t value initialized to the _end of the memory image.
largest_brk_ptr equ 0x40384c
sys_errlist equ 0x4028c0
fill_to 0x74, nop
syscall3:
	;movzx eax, al  ; Syscall number in AL. Looks like smart, but we have enough room for `mov eax, ...' below.
	push ebx  ; We have to save and restore EBX because the __cdecl calling convention requires. it.
	mov ebx, [esp+4+1*4]  ; arg1.
	mov ecx, [esp+4+2*4]  ; arg2.
	mov edx, [esp+4+3*4]  ; arg3.
	int 0x80  ; Linux i386 syscall.
	pop ebx
	test eax, eax
	jns .ret
	neg eax  ; Turn negative errno value to positive.
	mov dword [errno], eax
	or eax, byte -1  ; 
.ret:	ret
fill_to 0xa8, nop  ; _start
global _start
_start:
copy_to 0xc6  ; _exit
global _exit
_exit:
	pop ebx
	pop ebx  ; exit_code.
	xor eax, eax
	inc eax  ; SYS_exit.
	int 0x80
fill_to 0xd4, nop  ; procces_defines_in_argv.
copy_to 0x00c748  ; unlink.
global unlink
unlink:
	mov eax, 10
	jmp syscall3
fill_to 0x00c75c, nop
copy_to 0x00c75c  ; write.
global write
write:
	mov eax, 4
	jmp syscall3
fill_to 0x00c780, nop  ; time.
global time
time:
	mov eax, 13
	push ebx
	mov ebx, [esp+4]  ; tloc.
	int 0x80  ; Assume always success, don't check the return value.
	pop ebx
	ret
;copy_to 0x00c76f  ; handle_syscall_error.
;fill_to 0x00c76f+0x0e, copy
copy_to 0xd52a  ; Errno message strings.
errno_msgs:
e0	db 0
e1	db 'Operation not permitted', 0              ; EPERM == 1
e2	db 'No such file or directory', 0            ; ENOENT
e3	db 'No such process', 0                      ; ESRCH
e4	db 'Interrupted system call', 0              ; EINTR
e5	db 'I/O error', 0                            ; EIO
e6	db 'No such device or address', 0            ; ENXIO
e7	db 'Argument list too long', 0               ; E2BIG
e8	db 'Exec format error', 0                    ; ENOEXEC
e9	db 'Bad file number', 0                      ; EBADF
e10	db 'No child processes', 0                   ; ECHILD
e11	db 'Try again', 0                            ; EAGAIN
e12	db 'Out of memory', 0                        ; ENOMEM
e13	db 'Permission denied', 0                    ; EACCES
e14	db 'Bad address', 0                          ; EFAULT
e15	db 'Block device required', 0                ; ENOTBLK
e16	db 'Device or resource busy', 0              ; EBUSY
e17	db 'File exists', 0                          ; EEXIST
e18	db 'Cross-device link', 0                    ; EXDEV
e19	db 'No such device', 0                       ; ENODEV
e20	db 'Not a directory', 0                      ; ENOTDIR
e21	db 'Is a directory', 0                       ; EISDIR
e22	db 'Invalid argument', 0                     ; EINVAL
e23	db 'File table overflow', 0                  ; ENFILE
e24	db 'Too many open files', 0                  ; EMFILE
e25	db 'Not a typewriter', 0                     ; ENOTTY
e26	db 'Text file busy', 0                       ; ETXTBSY
e27	db 'File too large', 0                       ; EFBIG
e28	db 'No space left on device', 0              ; ENOSPC
e29	db 'Illegal seek', 0                         ; ESPIPE
e30	db 'Read-only file system', 0                ; EROFS
e31	db 'Too many links', 0                       ; EMLINK
e32	db 'Broken pipe', 0                          ; EPIPE
e33	db '?', 0                                    ; EDOM
e34	db '?', 0                                    ; ERANGE
e35	db '?', 0                                    ; EDEADLK
e36	db 'File name too long', 0                   ; ENAMETOOLONG
e37	db 'No record locks available', 0            ; ENOLCK
e38	db 'Function not implemented', 0             ; ENOSYS
e39	db 'Directory not empty', 0                  ; ENOTEMPTY
e40	db 'Too many symbolic links encountered', 0  ; ELOOP == 40
	; We have exactly 702 bytes for this, so the Linux errnos are
	; truncated here. Also some rare errors above are replaced with '?'.
copy_to 0x00e65b  ; _time.
fill_to 0x00e65b+0x15, nop
copy_to 0x00e670  ; close.
global close
close:
	mov eax, 6
	jmp syscall3
fill_to 0x00e684, nop
copy_to 0x00e684  ; lseek.
global lseek
lseek:
	mov eax, 19
	jmp syscall3
fill_to 0x00e698, nop
copy_to 0x00e698  ; sbrk.
global sbrk
sbrk:
	push ebx
	push ecx
	mov ecx, brk_end_ptr  ; Will remain so during the function.
	mov eax, 45  ; SYS_brk.
	mov ebx, [ecx]
	test ebx, ebx
	jne .no1st
	push eax
	xor ebx, ebx
	int 0x80  ; Linux i386 syscall.
	; Becaue of address randomization (ASLR), the memory region directly
	; after .bss may not be accessible. We call sys_brk(0) to get the lowest accessible address.
	; For more about ASLR, see
	; https://security.stackexchange.com/questions/229443/importance-of-aslr-mode-2
	mov [ecx], eax
	xchg eax, ebx  ; EBX := EAX; EAX := junk.
	pop eax  ; SYS_brk.
.no1st:	add ebx, [esp+2*4+4]  ; Argument increment.
	int 0x80  ; Linux i386 syscall.
	cmp ebx, eax
	jna .good
	or eax, -1  ; Indicate error. No need to set errno for our use case.
	pop ebx
	ret
.good:	xchg [ecx], ebx  ; Return the previous brk pointer. We set the new brk pointer to just what was asked, for for ASLR-consistency.
	xchg eax, ebx  ; EAX := EBX (result); EBX := junk.
	pop ecx
	pop ebx
	ret
;fill_to 0x00e6c2, nop  ; 6 bytes longer.
;copy_to 0x00e6c2  ; brk.
fill_to 0x00e6c2+0x1e, hlt
copy_to 0x00e81c  ; isatty.
global isatty
isatty:
	push ebx
	sub esp, strict byte 0x24
	xor eax, eax
	mov al,  54  ; SYS_ioctl.
	mov ebx, [esp+0x24+4+4]  ; fd argument of ioctl.
	mov ecx, 0x5401  ; TCGETS.
	mov edx, esp  ; 3rd argument of ioctl TCGETS.
	int 0x80  ; Linux i386 syscall.
	add esp, strict byte 0x24  ; Clean up everything pushed.
	pop ebx
	; Now convert result EAX: 0 to 1, everything else to 0.
	cmp eax, byte 1
	sbb eax, eax
	neg eax
	ret
fill_to 0x00e843, nop
copy_to 0x00ecf4  ; creat.
global creat
creat:
	mov eax, 8
	jmp syscall3
fill_to 0x00ed08, nop
copy_to 0x00ed08  ; getpid.
global getpid
getpid:
	mov eax, 20
	jmp syscall3
fill_to 0x00ed1b, nop
copy_to 0x00ed1b  ; ioctl.
fill_to 0x00ed1b+0x15, nop
copy_to 0x00ed30  ; kill.
global kill
kill:
	mov eax, 37
	jmp syscall3
fill_to 0x00ed43, nop
copy_to 0x00ed44  ; open.
global open
open:
	mov eax, 5
	jmp syscall3
fill_to 0x00ed58, nop
copy_to 0x00ed58  ; read.
global read
read:
	mov eax, 3
	jmp syscall3
fill_to 0x00ed6c, nop
fill_to 0x00edcc, nop  ; Don't copy unused_callback_setup_one_arg and whatever_callback.
copy_to 0x00ede4  ; End of .text.
text_end:

;section .data align=4
data:
y1 equ sys_errlist-data_base-0x00ede4
incbin 'as', 0x00ede4, y1
	;times 42 dd e1  ; Points to error lines.
	dd e0, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10, e11, e12, e13, e14, e15, e16
	dd e17, e18, e19, e20, e21, e22, e23, e24, e25, e26, e27, e28, e29, e30, e31
	dd e32, e33, e34, e35, e36, e37, e38, e39, e40
	dd e0  ; Useless for Linux, we support only 41 errno messages.
	dd 41  ; sys_nerr.
y2 equ brk_end_ptr-data_base-0x00ede4+4
incbin 'as', 0x00ede4+y1+43*4, y2-4-y1-43*4
dd 0  ; The is brk_end_ptr. First call to sbrk(...) will set it.
incbin 'as', 0x00ede4+y2, 0x001fbc-y2
data_end:

;section .bss align=4
;absolute $
bss_size equ 0xaac
;resb bss_size  ; From the COFF optional header in 'as'. https://wiki.osdev.org/COFF

; TODO:
;
; * DONE 0x0000a8 _start;; keep unchanged, because argc, argv and envp are passed the same way
; * DONE 0x0000c6 _exit;; use Linux syscall exit 1, no need to set the errno etc.
; * DONE 0x00ed58 read;; use Linux syscall read 3
; * DONE 0x00c75c write;; use Linux syscall write 4
; * DONE 0x00ed44 open;; use Linux syscall open 5;; only flag values 0..2 (O_RDONLY, O_WRONLY, O_RDWR) are used, and these are compatible between Linux and Coherent
; * DONE 0x00e670 close;; use Linux syscall close 6
; * DONE 0x00ecf4 creat;; use Linux syscall creat 8
; * DONE 0x00c748 unlink;; use Linux syscall unlink 10
; * DONE 0x00c780 time;; use Linux syscall time 13;; ignore the _time helper function, we don't need it; don't use the errno handling (jns)
; * DONE 0x00e6c2 brk;; don't implement it, it's unused
; * DONE 0x00e698 sbrk;; add a smart implementation based on Linux syscall brk 45
; * DONE 0x00e684 lseek;; use Linux syscall lseek 19
; * DONE 0x00ed08 getpid;; use Linux syscall getpid 20
; * DONE 0x00ed30 kill;; use Linux syscall kill 37;; the only signal number used is 6 for SIGABRT, that's compatible between Linux and Coherent
; * DONE 0x00e81c 0x28 bytes isatty;; implement it using Linux syscall ioctl 54, no need to set the errno for this caller
; * DONE 0x00ed1c ioctl;; don't implement it, it's used only from isatty
; * DONE Fixed sbrk(...) implementation, now it works with ASLR (even level 2).
; * DONE Moved .text (including .rdata) from memory base 0 to >=0x10000 (Modern Linux sysctl vm.mmap_min_addr = 65536 doesn't allow less.)
; * DONE Replaced as Coherent errno messages (pointed to by sys_errlist) with as many Linux messages as possible.
; * DONE Fixed .bss and .data offsets in the COFF .o output (fixcoff.pl).
; * Add linking this COFF output to an executable (folink3).
; * Add conversion from this COFF output to ELF-32 (first without symbols).

; More gaps in the program which we could use for adding more Linux-specific assembly code.
;
; * 0x000074 0x34 bytes: header size difference
; * 0x0000c6 0x0d bytes: _exit
; * 0x00c76f 0x0e bytes: handle_syscall_error
; * 0x00e65b 0x15 bytes: _time
; * 0x00e6c2 0x1e bytes: brk
; * 0x00ed1b 0x15 bytes: ioctl
; * 0x00ed6c many bytes after read
