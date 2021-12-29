CC=gcc
INCLUDES=-I/usr/src/libpri -I/usr/src/linux/include -I. -I./msu/libmsu
CFLAGS=-D__LINUX__ -D_REENTRANT -D_GNU_SOURCE -O6 -march=i686
CCFLAGS=-Wall -Wstrict-prototypes -Wmissing-prototypes -g

all: libsangoma.o 


libsangoma.o: libsangoma.c libsangoma.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS) -O2 -D_DEBUG_=2 -D_GNUC_ -I../lib -I/usr/src/linux/include -c -o libsangoma.o libsangoma.c

clean:
	rm -fr *.o *.so *~ *core* *.so* *.a
	@echo OK
