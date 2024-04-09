#! /bin/sh --
#
# compile.sh: compilation shell script for the Mark Williams 80386 assembler sources
# by pts@fazekas.hu at Tue Apr  9 05:22:53 CEST 2024
#
# Usage: ./compile.sh [<compiler-command> [<compiler-flag> ...]]
#
# It uses `gcc' by default is the compiler. See README.md for using other compilers.
#

set -ex

MYDIR=.
test "${0%/*}" = "$0" || MYDIR="${0%/*}"

ASC="main.c y_tab.c symtab.c lex.c common.c data.c space.c mwcvar.c cmd.c build.c sym.c hash.c c_out.c utype.c getargs.c _addargs.c strtod.c _pow10.c"
CDMPC="cdmp.c alloc.c xopen.c getargs.c"
TABBLDC="tabbld.c hash.c getline.c newcpy.c randl.c alloc.c xopen.c fatal.c utype.c getargs.c"

if test $# = 0; then
  set gcc
fi
CC="$1"
shift
CCBASE="${CC##*/}"
EXE=
IS_CROSS=  # If non-empty, then we do cross-compilation.
if test "$1" = --cross; then IS_CROSS=1; shift; fi
case "$CCBASE" in
 *tcc* | *tinycc*)  # TinyCC: https://bellard.org/tcc/
  CEXTRA="-O2 -W -Wall"
  if test "$CC" = miniutcc; then  # https://github.com/pts/minilibc686/blob/master/tools/miniutcc single-file precompiled for Linux i386, no #include.
    type "$CC" >/dev/null 2>&1 || CC=../tools/miniutcc
  fi
  test "${CCBASE#*miniutcc}" = "$CCBASE" || CEXTRA="$CEXTRA -Ic89include"
  ;;
 *owcc*)  # OpenWatcom (https://github.com/open-watcom/open-watcom-v2) targeting Linux i386 or Win32, running on Linux i386 or amd64.
  test "$WATCOM"
  CEXTRA="-fsigned-char -O2 -W -Wall -std=c89 -Wno-n308"  # We don't want -Werror.
  for ARG in "$@"; do
    test "$ARG" = -bwin32 && EXE=.exe
  done
  if test "$EXE"; then
    IS_CROSS=1
    set x -I"$WATCOM/h" -Wl,runtime -Wl,console=3.10 -march=i386 nouser32.c "$@"; shift  # Should be stripped. GNU strip(1) works.
  else
    set x -I"$WATCOM/lh" -blinux -march=i386 "$@"; shift
  fi
  ;;
 *)  # Defaults for GCC and Clang.
  CEXTRA="-O2 -W -Wall -ansi -pedantic" ;;  # We don't want -Werror.
esac

( cd "$MYDIR" &&
  "$CC" $CEXTRA $@ -o tabbld$EXE $TABBLDC &&
  (test "$IS_CROSS" || ./tabbld$EXE -d -e <table.386) &&   # Generates symtab.c, symtab.h, (-d not: document), (-e not: test.s). Can be skipped for cross-compilation.
  "$CC" $CEXTRA $@ -o as$EXE $ASC &&
  "$CC" $CEXTRA $@ -o cdmp$EXE $CDMPC &&
  :
) || exit "$?"
ls -l "$MYDIR/as$EXE" "$MYDIR/cdmp$EXE"

: "$0" OK.
