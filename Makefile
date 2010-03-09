O=chksum.o memory.o readfile.o crc.o
T=chksum
CC=g++
CFLAGS=-Wall
test: $T
	./$T bootloader_SWE1.mhx

chksum: $O
	$(CC) -o $T $O

clean:
	rm -f $T $O
