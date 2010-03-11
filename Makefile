T=memory
O=memory.o mem.o readfile.o writefile.o crc.o var.o
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

# DO NOT DELETE
