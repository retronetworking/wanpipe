################################################################################
# Sangoma MGD
#
# Author:   Anthony Minessale II <anthmct@yahoo.com>
#           Nenad Corbic <ncorbic@sangoma.com>
#
# Copyright:   (c) 2005 Anthony Minessale II
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
################################################################################

SMG_DTMF=YES

CC = gcc
INCLUDES = -I/lib/modules/$(shell uname -r)/build/include -I. -I/usr/local/include -I../../patches/kdrivers/include -I/usr/include/wanpipe
CFLAGS = -D__LINUX__ -D_REENTRANT -D_GNU_SOURCE -O6 -march=i686
CCFLAGS = -Wall -Wstrict-prototypes -Wmissing-prototypes -g
LDFLAGS=-L. -L/usr/local/lib -lpthread -lsangoma -lm 

ifeq "${SMG_DTMF}" "YES"
LDFLAGS+= -lteletone
CFLAGS+= -DSMG_DTMF_ENABLE
endif


all: sangoma_mgd 


switch_buffer.o: switch_buffer.c switch_buffer.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o switch_buffer.o switch_buffer.c

call_signal.o: call_signal.c call_signal.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o call_signal.o call_signal.c

sangoma_mgd: sangoma_mgd.o call_signal.o switch_buffer.o sigboost.h
	rm -fr core*
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o sangoma_mgd sangoma_mgd.o switch_buffer.o call_signal.o $(LDFLAGS)

sangoma_mgd.o: sangoma_mgd.c sangoma_mgd.h sigboost.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS) -c -o sangoma_mgd.o sangoma_mgd.c

clean:
	find . -name '*.o' | xargs rm -f
	rm -fr sangoma_mgd pritest *.o *.so *~ *core* *.so* *.a

distclean: clean
	@echo OK

install: all
	/bin/cp sangoma_mgd /usr/sbin
	test -f /etc/sangoma_mgd.conf || /bin/cp sangoma_mgd.conf.sample /etc/sangoma_mgd.conf
	@echo "sangoma_mgd Installed"

uninstall: 
	/bin/rm /usr/sbin/sangoma_mgd /etc/sangoma_mgd.conf
