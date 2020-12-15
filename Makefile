# intmul by Johannes Zottele 11911133

CC = gcc
compile_flags = -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_BSD_SOURCE -D_SCID_SOURCE -D_POSIX_C_SOURCE=200809L -g

all: intmul

hexmul: intmul.o hexcalc.o
	$(CC) $(compile_flags) -o $@ $^

intmul: intmul.o hexcalc.o
	$(CC) $(compile_flags) -o $@ $^

%.o: %.c
	$(CC) $(compile_flags) -c -o $@ $<

# intmul.o: intmul.c intmul.h	
hexcalc.o: hexcalc.c hexcalc.h

clean:
	rm -rf *.o intmul hexmul