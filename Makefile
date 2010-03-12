T=memory
O=memory.o mem.o readfile.o writefile.o crc.o var.o lex.o parse.o fn.o
CC=g++ -Wall -g

test: $T
	./$T script.mem


$T: $O
	$(CC) -o $T $O

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -f $T $O *~ *.stackdump *.bak

archive:
	git archive --format=zip HEAD > $T.zip

depend:
	makedepend -Y *.cpp

# DO NOT DELETE

crc.o: mem.h
fn.o: parse.h mem.h lex.h var.h fn.h
lex.o: lex.h mem.h var.h
mem.o: mem.h
memory.o: mem.h parse.h lex.h var.h
parse.o: parse.h mem.h lex.h var.h fn.h
readfile.o: mem.h
var.o: var.h mem.h lex.h
writefile.o: mem.h
