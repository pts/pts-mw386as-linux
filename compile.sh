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

# From Coherent 4.2.10, -r-x--x--x 1 pts pts 69024 Nov 11  1992 as
if ! test -f as; then
  if ! test -f ddk32.dd; then
    if test -f "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd"; then
      tools/mv "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd" ./
    else
      if ! test -f "Coherent Unix 4.2.10 (1994) (3.5-1.44mb).7z"; then
        : 'Recommended download: https://www.tuhs.org/Archive/Distributions/Other/Coherent/base/ddk32.dd'
        : 'Alternative download "Coherent Unix 4.2.10 (1994) (3.5-1.44mb).7z" from https://winworldpc.com/download/532ebb91-483c-11ed-b2b1-0200008a0da4'
        exit 1
      fi
      7z x -y "Coherent Unix 4.2.10 (1994) (3.5-1.44mb).7z" "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd" ||:
      if ! test -f "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd"; then
        tools/tiny7zx x -y "Coherent Unix 4.2.10 (1994) (3.5-1.44mb).7z" ||:
      fi
      if ! test -f "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd"; then
        : "Extraction failed."
        exit 1
      fi
      tools/mv "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)/base/ddk32.dd" ./
      tools/rm -rf "Coherent Unix 4.2.10 (1994) (3.5-1.44mb)"
    fi
  fi
  test -f ddk32.dd
  # Extract single file as form disk image ddk32.dd.
  tools/miniperl-5.004.04 -e '
      BEGIN { $^W = 1 } use integer; use strict;
      $_ = "000102030405060708090b0c0d0e0f101112131415161718191a1b1c1d
      1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d
      3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d
      5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d
      7e7f8081828384858687"; s@\s+@@g; $_ = pack("H*", $_); my $s;
      my $size = 69024; my $try; binmode(STDIN); binmode(STDOUT);
      s@(.)@
        die 3 if !seek(STDIN, (ord($1) + 1123) << 9, 0);
        $try = ($size > 0x200 ? 0x200 : $size);
        die 4 if read(STDIN, $s, $try) != $try;
        die 5 if !print($s);
        $size -= length($s);
      @gse; die$@if$@' <ddk32.dd >as
fi

old_path="$PATH"
export PATH=/dev/null/nopath

# From Coherent 4.2.10, -r-x--x--x 1 pts pts 69024 Nov 11  1992 as
test -f as
tools/miniperl-5.004.04 -x check.pl as  # Check size and checksum.

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
tools/miniperl-5.004.04 -x link3coff.pl --elf hellol.o hellol
./hellol  # Prints `Hello, World!'.
test "$(./hellol)" = "Hello, World!"
tools/miniperl-5.004.04 -x link3coff.pl --elf2 hellol.o hellol
./hellol  # Prints `Hello, World!'.
test "$(./hellol)" = "Hello, World!"
tools/miniperl-5.004.04 -x fixcoff.pl hellol.o
tools/miniperl-5.004.04 -x link3coff.pl --elf hellol.o hellol
./hellol  # Prints `Hello, World!'.
test "$(./hellol)" = "Hello, World!"

if test $# != 0; then  # Run some more tests. GNU Binutils is needed.
  PATH="$old_path"  # For objdump and ld.
  # Tested with objdump(1) and ld(1) in GNU Binutils 2.24 on Debian.
  # Some custom-compiled ld(1) linkers don't have COFF .o support, and they fail with: hellol.o: file not recognized: File format not recognized
  objdump -d hellol.o
  ld -m elf_i386 -o hellol hellol.o  # Also works with -N (--elf). Without -N it's --elf2.
  ./hellol  # Prints `Hello, World!'.
  test "$(./hellol)" = "Hello, World!"
fi

tools/rm -f mw386as
tools/cp -a asl5 mw386as
tools/ls -ld mw386as

: "$0" OK.
