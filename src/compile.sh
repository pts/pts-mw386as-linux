#! /bin/sh --
#
# compile.sh: compilation shell script for the Mark Williams 80386 assembler sources
# by pts@fazekas.hu at Tue Apr  9 05:22:53 CEST 2024
#
# Usage: ./compile.sh [<compiler-command> [<compiler-flag> ...]]
#
# It uses `gcc' by default is the compiler
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
case "$CCBASE" in
 *tcc* | *tinycc*)  # TinyCC: https://bellard.org/tcc/
  CEXTRA="-O2 -W -Wall" ;;
 *owcc*)  # OpenWatcom (https://github.com/open-watcom/open-watcom-v2) targeting Linux i386, running on Linux i386 or amd64.
  test "$WATCOM"
  set x -I"$WATCOM/lh"; shift
  CEXTRA="-blinux -fsigned-char -O2 -W -Wall -Werror -std=c89 -Wno-n308" ;;
 *)  # Defaults for GCC and Clang.
  CEXTRA="-O2 -W -Wall -ansi -pedantic" ;;  # No -Werror.
esac

( cd "$MYDIR" &&
  "$CC" $CEXTRA $@ -o tabbld $TABBLDC &&
  ./tabbld -d -e <table.386 &&   # Generates symtab.c, symtab.h, (-d not: document), (-e not: test.s). Can be skipped for cross-compilation.
  "$CC" $CEXTRA $@ -o as $ASC &&
  "$CC" $CEXTRA $@ -o cdmp $CDMPC &&
  :
) || exit "$?"
ls -l "$MYDIR/as" "$MYDIR/cdmp"

: "$0" OK.
