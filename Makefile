T=chksum
O=chksum.o memory.o readfile.o writefile.o crc.o
CC=g++ -Wall -g

test: $T
	./$T script.chk


chksum: $O
	$(CC) -o $T $O

.cpp.o:
	$(CC) $(CFLAGS) -c $< 

clean:
	rm -f $T $O *.stackdump

archive:
	git archive -o chksum.zip HEAD

crc.o: memory.h 
memory.o: memory.h 
readfile.o: memory.h
writefile.o: memory.h
chksum.o: memory.h 
