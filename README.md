# pts-mw386as-linux: ports of the Mark Williams 80386 assembler

pts-mw386as-linux consits of ports of the Mark Williams 80386 assembler to
modern platforms. Originally, in 1992--1993, the assembler was written for
Coherent (a Unix clone) running on i386 systems. The assembler was released
as open source (3-clause BSD license) on 2015-01-03, as part of
[open sourcing Coherent](http://www.nesssoftware.com/home/mwc/source.php).
The assembler can read its own AT&T assembly syntax
similar to GNU as(1), and can generate a COFF object file. The assembler
supports 16-bit and 32-bit mode, and it knows most Intel 386 instructions
and addressing modes.

## The binary ports

Some of the ports in pts-mw386as-linux are direct Linux i386 ports of the
released Coherent i386 executable program binaries. Functionality is
unchanged. The ported assembler *mw386as* runs natively on Linux x86 (i386
and amd64) systems as a Linux i386 console program. No other files are
required, the Linux port of the assembler is self-contained and statically
linked (libc-independent).

One of the binary ports is a port of the 1992-11-11 binary release of the
*as* executable program (69024 bytes), part of Coherent 4.2.10. To compile
that (i.e. convert it to a Linux i386 executable), just run `./compile.sh`
on a Linux x86 (i386 or amd64) system. (If you need to download additional
files, it will tell you so, and then run `./compile.sh` again.) The output
file will be *mw386as*, which has the same size as the original *as* file,
but the Coherent-specific parts are binary-patched to be specific to Linux
i386 instead.

Compiling the *mw386as* executable program is self-contained. All the needed
tools are included in the repository (in the directory `tools/`) as
precompiled Linux i386 executables. These tools are: BusyBox (including a
shell), Perl, NASM and tiny7zx. The build process uses shell scripts, Perl
scripts and NASM source files as sources.

Some other binary releases of the Coherent *as* executable program have also
been released in 1992 and 1993, but those haven't been ported to Linux in
this repository yet. The earliest known one is from 1992-09-11, and it's
within the
[as,v](https://github.com/gspu/Coherent/blob/master/mwc/romana/relic/b/bin/as/RCS/as%2Cv)
RCS file.

## The source port

Another port in this repository is the port of the C source code of the Mark
Williams 80386 assembler to C89 (ANSI C). It's in the directory `src/`. To
compile it on a Unix system (e.g. Linux or macOS), install a C compiler, and
run the script `src/compile.sh`. The output executables are `src/as`
(assembler) and `src/cdmp` (with similar purpose as GNU objdump(1), but for
COFF object files).

The script will compile with GCC by default, but you can specify other
compliers, like this:

* `src/compile.sh` is the same as `./compile.sh gcc`.

* `src/compile.sh gcc` uses GCC as the compiler. You can also specify flags
  like `src/compile.sh gcc -std=c99 -Werror`. Instead of `gcc`, you can
  specify any GCC command installed, e.g. `src/compile.sh gcc-7`. You can
  also specify `--cross` right after `gcc` to enable cross-compilation. That
  will prevent the rebuilding of *symtab.c*.

* `src/compile.sh clang` uses Clang as the compiler, otherwise it's similar
  to `src/compile.sh gcc`.

* `src/compile.sh tcc` uses [TinyCC](https://bellard.org/tcc/) as the
  compiler. The script adds different warning flags (e.g. no `-ansi`) than
  for GCC, but otherwise it's similar to `src/compile.sh gcc`. Please note
  that TinyCC is a fast, but non-optimizing compiler: the generated execable
  is slow and big.

* `src/compile.sh miniutcc` uses *miniutcc* as the C compiler, which is a
  self-contained variant of [TinyCC](https://bellard.org/tcc/). It's part of
  the pts-mw386as-linux Git repository as the Linux i386 executable
  `tools/miniutcc`. It's self-contained, installing any C compiler, C header
  files, libc, assembler or linker is not necessary. It generates
  statically linked Linux i386 executables.

* `src/compile.sh owcc` uses the
  [OpenWatcom](https://github.com/open-watcom/open-watcom-v2) C compiler and
  targets Linux i386. It generates a statically linked ELF-32 executable.

* `src/compile.sh owcc -bwin32` uses the OpenWatcom C compiler and targets
  Win32 (Windows i386). The generated *as.exe* uses only kernel32.dll and
  nothing else (e.g. no user32.dll or msvcrt.dll).

* Usually there is no need to specify extra compiler flags, the defaults
  provided by the script are good.

* It is also straightforward to compile it on Windows or target Windows or
  target other systems, but there are no instructions or scripts provided.

The source port is based on the [latest
sources](https://github.com/gspu/Coherent/tree/master/mwc/romana/relic/b/bin/as)
of the assembler (1993-08-02), These sources are also part of the official
2015 Coherent official release tarball
[mwc.tgz](http://www.nesssoftware.com/home/mwc/mwc.tgz), they are in the
*romana/relic/b/bin/as* directory. The pts-mw386as-linux Git repository
retains the full development history (1992--1993) of the source code, the
file revisions were imported from RCS (also in the tarball).

The source port makes the following changes:

* It changes old C syntax to C89 (still old, but at least standard, so many
  C compilers support it). Mostly replaces `<stdarg.h>`, it fixes function
  prototypes, and it adds function declarations.
* It replaces calls to Coherent-specific functions with standard C library
  functions.
* It replaces the Coherent-specific printf(3) format specifier `%r`.
* It adds the implementations of some non-C89 libc functions, so the code
  compiles witha C89 libc only.
* It adds the necessary portability changes for 64-bit systems (such as
  amd64).
* It makes the program work either if char is signed or unsigned.
* It fixes tons of C compiler warnings. (There shouldn't be any more left.)
* It fixes some bugs (but the fixes don't change the output object file.)
* It makes the program compile in C++98 (this is not finished yet) as well
  as C89.
* It adds general portability improvements for more C compilers and systems.

## How to use the assembler

The Mark Williams 80386 assembler reads assembly source files in its own
syntax very similar to AT&T (and thus GNU as(1)), and it produces COFF
object files. In 1992 and 1993 it was a goal to use the assembler as a
backend for GCC on Coherent, so it understands everything generated by GCC
at that time.

The ld(1), objdump(1) etc. tools from GNU Binutils on modern Linux still
understand COFF object files as input, but *mw386as* generates a different
flavour of COFF. A converter Perl script *fixcoff.pl* is provided by this
port. Alternatively, a simple linker Perl script *link3coff.pl* is also
provided, which can build ELF-32 Linux i386 executables directly from the
output of the assembler. See the file *hellol.s* for a hello-world assembly
program for Linux i386, with compilation instructions. For comparison, the
same program is provided in GNU as(1) syntax in the file *hellola.s*.
