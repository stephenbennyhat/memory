O=chksum.o memory.o readfile.o writefile.o crc.o
T=chksum
CC=g++ -Wall -g


test: $T
	./$T bootloader_SWE1.mhx

chksum: $O
	$(CC) -o $T $O

clean:
	rm -f $T $O

.cpp.o:
		$(CC) $(CFLAGS) -c $< 

chksum.o: memory.h readfile.h crc.h
crc.o: memory.h crc.h
memory.o: memory.h crc.h
readfile.o: memory.h readfile.h
writefile.o: memory.h
