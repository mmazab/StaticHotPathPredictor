# Don't do -D__inline__= as this bones sys/stat.h
CFLAGS=-DPRINT_CONFLICTS -DPRIEPSET -D_GNU_SOURCE -D_XOPEN_SOURCE=600 -Wall -pedantic -Wno-long-long -g -O0 -I.
CCFLAGS=-D_XOPEN_SOURCE=600 -Wall -pedantic -Wno-long-long -g -O1 -I. -std=c++0x

%.o : %.cxx
	g++ -c $(CCFLAGS) -o $@ $<

%.o : %.c
	gcc -c $(CCFLAGS) -o $@ $<

%.o : %.S
	gcc -c $(CFLAGS) -o $@ $<

all: MemoryMap.o debug_new.o utils.o

clean:
	rm -rf *.o
 
