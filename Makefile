T=memory
O=memory.o mem.o readfile.o writefile.o crc.o
CC=g++ -Wall -g

test: $T
	./$T script.mem


$T: $O
	$(CC) -o $T $O

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -f $T $O *~ *.stackdump

archive:
	git archive -o $T.zip HEAD

crc.o: memory.h 
memory.o: memory.h 
readfile.o: memory.h
writefile.o: memory.h
chksum.o: memory.h 
