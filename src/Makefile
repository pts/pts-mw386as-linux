# Object files required directly by as.386
O = main.o y_tab.o symtab.o lex.o common.o data.o space.o mwcvar.o \
 cmd.o build.o sym.o hash.o c_out.o utype.o getargs.o

# Files required to build as.386 which are not scanned for comments
B=Makefile fixy tabbld.c getline.c table.386 docfix.c fixdoc.c script \
    mwcvar.c

# Files containing comments
S = utype.h asm.y lex.c common.c data.c strtod.c _pow10.c cmd.c build.c sym.c \
 hash.c c_out.c utype.c space.c asfix.c main.c getargs.c

# Header files
H = asm.h asme.h asflags.h

# Constructed header files
J = y_tab.h symtab.h

# Files required to build as.386 on the Atari
K = main.c utype.h tabbld.c table.386 y_tab.c lex.c common.c data.c space.c \
 strtod.c _pow10.c cmd.c build.c sym.c hash.c c_out.c utype.c  mwcvar.c \
 getargs.c ieee_f.c ieee_d.c getline.c asfix.c y_tab.h $H

I =  -I.. -I.
CFLAGS = -A $I

as : $O
	cc -so as $O

install : as
	su root `srcpath installer`

save : $H $B $S
	tar cf - $< | compress -c > /tmp/a386.tar.Z
	uucp -r /tmp/a386.tar.Z mwc!/v/cef/

ker : $K
	kas $?
	touch ker

doc : $S
	cgrep -d @ "(yyerror|yywarn|fatal)" $< | \
	 sed -f `srcpath script` | \
	 sort | \
	 fixdoc > doc

list : $H $B $S
	ph  $?
	touch list

fixdoc : fixdoc.c
	cc -A $<

asfix : asfix.c
	cc -A $<

y_tab.o : y_tab.c $H

y_tab.c y_tab.h: asm.y
	yacc $<
	mv y.tab.c y_tab.c
	mv y.tab.h y_tab.h
	ed y_tab.c < `srcpath fixy`

lex.o: lex.c $J $H utype.h
tabbld: tabbld.c hash.o getline.o
	cc -A $< -lmisc

getline.o : getline.c
utype.o : utype.c utype.h
c_out.o : c_out.c utype.h
build.o : build.c $J $H
main.o : main.c $H
sym.o : sym.c $J $H
hash.o : hash.c
common.o: common.c $H
data.o : data.c $H
space.o : space.c $H
mwcvar.o : mwcvar.c $H
cmd.o : cmd.c $J $H
getargs.o : getargs.c

docfix : docfix.c
	cc $<

symtab.c symtab.h: table.386 tabbld docfix
	tabbld < `srcpath table.386`
	sort document | docfix > opcodes.doc
	rm document
	egrep -v "(and|or|xor)(w|l).*0x64," test.s > t.s

symtab.o : symtab.c $J $H
	cc -c $I symtab.c
