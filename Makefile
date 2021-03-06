T=memory
O=memory.o mem.o readfile.o writefile.o crc.o var.o lex.o parse.o fn.o memutils.o trace.o
CC=g++ -Wall -g -DUSE_TR1

$T: $O
	$(CC) -o $T $O

.PHONY: test
test:$T
	cd test && ./runtest

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -f $T $O *~ *.stackdump *.bak $T.zip

archive:
	git archive --format=zip HEAD > $T.zip

depend:
	makedepend -Y *.cpp

# DO NOT DELETE

crc.o: mem.h memtypes.h range.h
fn.o: parse.h port.h mem.h memtypes.h range.h lex.h var.h fn.h
lex.o: lex.h mem.h memtypes.h range.h var.h port.h
mem.o: mem.h memtypes.h range.h
memory.o: trace.h lex.h mem.h memtypes.h range.h parse.h port.h var.h
memutils.o: mem.h memtypes.h range.h
parse.o: parse.h port.h mem.h memtypes.h range.h lex.h var.h fn.h trace.h
readfile.o: mem.h memtypes.h range.h
trace.o: trace.h lex.h mem.h memtypes.h range.h
var.o: var.h port.h mem.h memtypes.h range.h lex.h
writefile.o: mem.h memtypes.h range.h
