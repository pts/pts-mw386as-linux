# Oversimplified Makefile to compile the assembler from C sources.
# For a smarter alternative, use compile.sh (a Unix shell script) instead.

.PHONY: all clean

CC = cc
CFLAGS =
H = asflags.h asm.h asme.h coff.h intsize.h utype.h y_tab.h
ASC = main.c y_tab.c symtab.c lex.c common.c data.c space.c mwcvar.c cmd.c build.c sym.c hash.c c_out.c utype.c getargs.c _addargs.c strtod.c _pow10.c
CDMPC = cdmp.c alloc.c xopen.c getargs.c
TABBLDC = tabbld.c hash.c getline.c newcpy.c randl.c alloc.c xopen.c fatal.c utype.c getargs.c

all: as cdmp

as: $(ASC) $(H) symtab.h
	$(CC) $(CFLAGS) -o as $(ASC)
cdmp: $(CDMPC) $(H)
	$(CC) $(CFLAGS) -o cdmp $(CDMPC)
tabbld: $(TABBLDC) $(H)
	$(CC) $(CFLAGS) -o tabbld $(TABBLDC)

# Remove this rule for cross-compilation. Then the pregenerated symtab.c and
# symtab.h in the repository will be used.
symtab.c symtab.h: tabbld table.386
	# Generates symtab.c, symtab.h, (-d not: document), (-e not: test.s). Can be skipped for cross-compilation.
	./tabbld -d -e <table.386

clean:
	rm -f as cdmp tabbld ./*.o
