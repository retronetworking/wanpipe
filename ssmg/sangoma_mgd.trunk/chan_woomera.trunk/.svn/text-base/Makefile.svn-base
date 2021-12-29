################################################################################
# chan_woomera: Woomera channel driver for Asterisk/CallWeaver
#
# Author:   Nenad Corbic <ncorbic@sangoma.com>
#	    Anthony Minessale II <anthmct@yahoo.com>
#
# Copyright:   (c) 2007 Nenad Corbic <ncorbic@sangoma.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version
# 2 of the License, or (at your option) any later version.
################################################################################

ifndef PBXDIR
	PBXDIR=/usr/src/asterisk
ifneq (,$(wildcard ./.pbxdir))
	PBXDIR=$(shell cat .pbxdir)
endif
endif

$(shell echo $(PBXDIR) > .pbxdir)	

PBXMODDIR=/usr/lib/asterisk/modules
PBXCFGDIR=/etc/asterisk

ifneq (,$(wildcard $(PBXDIR)/include/callweaver.h))
	PBXFLAGS_EXTRA=-DCALLWEAVER -DHAVE_CONFIG_H
	PBXMODDIR=/usr/local/lib/callweaver/modules
	PBXCFGDIR=/usr/local/etc/callweaver
endif

ifneq (,$(wildcard $(PBXDIR)/include/asterisk.h))
ifneq (,$(wildcard $(PBXDIR)/configure))
	PBXFLAGS_EXTRA=-DAST14 -DAST_MODULE=\"chan_woomera\" 
else
	PBXFLAGS_EXTRA=
endif
endif


CC = gcc

INCLUDES= -I/usr/include -I./

CFLAGS = -D__LINUX__ -D_REENTRANT -D_GNU_SOURCE -O6 -march=i686
CCFLAGS = -Wall -Wstrict-prototypes -Wmissing-prototypes -g
LDFLAGS=-L lib/libteletone/.libs -L. -L/usr/local/lib -L ../../ssmg/libsangoma.trunk/.libs -lpthread -lsangoma -lm

PBXFLAGS= $(INCLUDES) -I$(PBXDIR) -I$(PBXDIR)/include -pipe  -Wall -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -g3 -Iinclude -I../include -D_REENTRANT -D_GNU_SOURCE  -O6 -march=i686 -fomit-frame-pointer  -fPIC


all: chan_woomera.so 

chan_woomera.so: chan_woomera.o
	$(CC) -shared -Xlinker -x -o chan_woomera.so chan_woomera.o

chan_woomera.o: chan_woomera.c
	$(CC)  $(PBXFLAGS) $(PBXFLAGS_EXTRA) -c chan_woomera.c -o chan_woomera.o

clean:  
	find . -name '*.*o' | xargs rm -f

distclean: clean
	@echo OK

install: all  
	install -D -m 755 chan_woomera.so $(INSTALLPREFIX)$(PBXMODDIR)/chan_woomera.so
	@if [ -f woomera.conf ] && [ ! -e $(INSTALLPREFIX)$(PBXCFGDIR)/woomera.conf ]; then \
		install -D -m 755 woomera.conf $(INSTALLPREFIX)$(PBXCFGDIR)/woomera.conf; \
	fi
	@echo "chan_woomera Installed"

