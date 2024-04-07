#! /bin/sh --
#
# compile.sh: build the Mark Williams 80386 assembler for Linux i386
# by pts@fazekas.hu at Sun Apr  7 08:43:16 CEST 2024
#
# Input file: as, the Mark Williams 80386 assembler executable for Coherent,
# as shipped with Coherent 4.2.10.
#
# Output file: asl5, the equivalent Linux i386 executable.
#
# There are some other temporary output files as well.
#

export LC_ALL=C
test "$0" = "${0%/*}" || cd "${0%/*}"
if test "$1" != --sh-script; then
  exec tools/busybox sh "$0" --sh-script "$@"
  exit 1
fi
shift

set -ex

test $# = 0 && PATH=/dev/null/nopath

# From Coherent 4.2.10, -r-x--x--x 1 pts pts 69024 Nov 11  1992 as
test -f as
#tools/miniperl-5.004.04 -x check.pl as  # Check size and checksum.

tools/nasm-0.98.39 -O999999999 -w+orphan-labels -f bin -o asl4 asl4.nasm
tools/chmod +x asl4
# It's already possible to run asl4 as: qemu-i386 asl4
#
# It doesn't work as `./asl4' on modern Linux i386, because Linux has sysctl
# vm.mmap_min_addr = 65536, and asl4 maps its .text (and .rodata) to address
# 0 (too low). We fix it by building asl5, where we relocate .text.

tools/miniperl-5.004.04 -x patch.pl asl4 asl5 0x3f1000
#tools/miniperl-5.004.04 -x patch.pl asl4 asl5 0x300000  # For easy debugging.
tools/chmod +x asl5

exit_code=0; ./asl5 || exit_code="$?"
test "$exit_code" = 1
test "$(./asl5)" = "no work"
test "$(./asl5 -V 2>&1 >/dev/null)" = "Mark Williams 80386 assembler"
exit_code=0; ./asl5 /dev/null/missing || exit_code="$?"
test "$exit_code" = 1
test "$(./asl5 /dev/null/missing 2>&1 >/dev/null)" = "errno reports: Not a directory"
test "$(./asl5 /dev/null/missing 2>/dev/null)" = "Cannot fopen(/dev/null/missing, r)"
tools/rm -f hellol.o  # Works without it.
./asl5 hellol.s

if test $# != 0; then  # Run some more tests. GNU Binutils is needed.
  objdump -d hellol.o
  ld -m elf_i386 -N -o hellol hellol.o  # !! .bss size is broken in COFF, we don't care.
  ./hellol  # Prints `Hello, World!'.
  test "$(./hellol)" = "Hello, World!"
fi

: "$0" OK.
