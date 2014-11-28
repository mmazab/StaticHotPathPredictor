# Don't do -D__inline__= as this bones sys/stat.h
CFLAGS=-D_GNU_SOURCE -D_XOPEN_SOURCE=600 -c -Wall -pedantic -Wno-long-long -g -O0 -I. -std=c++0x

lamp_hooks.o: lamp_hooks.cxx
	g++ $(CFLAGS) -o $@ $<

all:  lamp_hooks.o

clean:
	rm -rf *.o
