# pts-mw386as-linux: a port of the Mark Williams 80386 assembler to Linux i386

pts-mw386as-linux is a port of the Mark Williams 80386 assembler
(1992-11-11, part of Coherent 4.2.10) to modern Linux i386. Functionality is
unchanged, the assembler can read its own AT&T assembly syntax similar to
GNU as(1), and generate a COFF object file. The assembler supports 16-bit
and 32-bit mode, and it knows most Intel 386 instructions and addressing
modes. The ported assembler *mw386as* runs natively on Linux x86 (i386 and
amd64) systems as a Linux i386 console program. No other files are required,
the Linux port of the assembler is self-contained and statically linked
(libc-independent).

The ld(1), objdump(1) etc. tools from GNU Binutils on Linux understand COFF
object files as input, but *mw386as* generates a different flavour of COFF.
A converter Perl script *fixcoff.pl* is provided. Alternatively, a simple
linker Perl script *link3coff.pl* is also provided, which can build ELF-32
Linux i386 executables directly from the output of *mw386as*. See the file
*hellol.s* for a hello-world assembly program and compilation instructions.
For perspective, the same program is provided in GNU as(1) syntax in the
file *hellola.s*.

To build *mw386as*, you need a copy of the original *as* file (69024 bytes)
from Coherent 4.2.10 first. Just run `./compile.sh` on a Linux x86 system,
and it will tell you where to download *as* from. Then run `./compile.sh` to
build and test the *mw386as* executable, which is the same size as the
original *as* file, but the Coherent-speficic parts are binary-pathed to be
specific to Linux i386 instead.

An alternative executable from 1992-09-11 has been open sourced as part of
the
[as,v](https://github.com/gspu/Coherent/blob/master/mwc/romana/relic/b/bin/as/RCS/as%2Cv)
RCS file. This hasn't been ported to Linux i386 yet.
