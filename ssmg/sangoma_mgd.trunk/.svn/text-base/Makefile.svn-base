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

#Default kernel directory to be overwritten by user
#Kernel version and location
ifndef KVER
       KVER=$(shell uname -r)
endif
ifndef KMOD
       KMOD=/lib/modules/$(KVER)
endif
ifndef KDIR
       KDIR=$(KMOD)/build
endif
ifndef KINSTDIR
       KINSTDIR=$(KMOD)/kernel
endif

CC = gcc
INSTALLPREFIX=

INCLUDES = -I ../../ssmg/libsangoma.trunk -I. -I ../../patches/kdrivers/include -I ../../patches/kdrivers/wanec/oct6100_api/include -I ../../patches/kdrivers/wanec -I/usr/local/include -I../../patches/kdrivers/include -I/usr/include/wanpipe -Ilib/libteletone/src

CFLAGS = -D__LINUX__ -D_REENTRANT -D_GNU_SOURCE -O6 

ifeq "${BRI}" "YES"
CFLAGS += -DBRI_PROT -DSMG_CALLING_NAME
NO_SS7:=YES
else
ifeq "${NO_SS7}" "YES"
BRI:=YES
CFLAGS += -DBRI_PROT
endif
endif

CCFLAGS = -Wall -Wstrict-prototypes -Wmissing-prototypes -g
LDFLAGS=-L lib/libteletone/.libs -L. -L/usr/local/lib -L ../../ssmg/libsangoma.trunk/.libs -lpthread -lsangoma -lm

#Enable memory leak subsystem
#Not to be used in production
#CFLAGS += -DSMG_MEMORY_DEBUG


ifeq "${SMG_DTMF}" "YES"
LDFLAGS+= -lteletone
CFLAGS+= -DSMG_DTMF_ENABLE
endif


all: sangoma_mgd  

libs: 
	$(shell cd lib/libteletone; ./configure --prefix=$(INSTALLPREFIX); cd ../../; )
	$(MAKE) -C lib/libteletone all

switch_buffer.o: switch_buffer.c switch_buffer.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o switch_buffer.o switch_buffer.c

call_signal.o: call_signal.c call_signal.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o call_signal.o call_signal.c

sangoma_mgd_memdbg.o: sangoma_mgd_memdbg.c sangoma_mgd_memdbg.h 
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o sangoma_mgd_memdbg.o sangoma_mgd_memdbg.c

sangoma_mgd_logger.o: sangoma_mgd_logger.c 
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS)  -c -o sangoma_mgd_logger.o sangoma_mgd_logger.c

sangoma_mgd.o: sangoma_mgd.c sangoma_mgd.h sigboost.h
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS) -c -o sangoma_mgd.o sangoma_mgd.c 

sangoma_mgd: sangoma_mgd.o sangoma_mgd_memdbg.o  sangoma_mgd_logger.o call_signal.o switch_buffer.o sigboost.h sangoma_mgd_memdbg.h
	rm -fr core*
	$(CC) $(CCFLAGS) $(INCLUDES) $(CFLAGS) -o sangoma_mgd sangoma_mgd.o sangoma_mgd_memdbg.o  sangoma_mgd_logger.o switch_buffer.o  call_signal.o $(LDFLAGS)



clean:  old_cleanup
	find . -name '*.o' | xargs rm -f
	rm -fr sangoma_mgd pritest *.o *.so *~ *core* *.so* *.a
	make -C lib/libteletone clean

distclean: clean
	@echo OK

install: all install_smg old_cleanup

install_smg: old_cleanup 
	install -D -m 755 sangoma_mgd $(INSTALLPREFIX)/usr/sbin/sangoma_mgd
	@if [ ! -e $(INSTALLPREFIX)/etc/sangoma_mgd.conf ]; then \
		install -D -m 755 sangoma_mgd.conf.sample $(INSTALLPREFIX)/etc/sangoma_mgd.conf; \
	fi
	
	install -D -m 755 ./safe_sangoma $(INSTALLPREFIX)/usr/sbin/safe_sangoma
	install -D -m 755 ./__smg_ctrl_common $(INSTALLPREFIX)/usr/sbin/__smg_ctrl_common
ifeq "${BRI}" "YES"
	@echo "BRI control scripts installed"
	@if [ ! -e $(INSTALLPREFIX)/etc/sangoma_mgd.conf ]; then \
		install -D -m 755 sangoma_mgd.conf.sample.bri $(INSTALLPREFIX)/etc/sangoma_mgd.conf; \
	fi
	install -D -m 755 smg_ctrl_bri $(INSTALLPREFIX)/usr/sbin/smg_ctrl
else
	@echo "SS7 control scripts installed"
	@if [ ! -e $(INSTALLPREFIX)/etc/sangoma_mgd.conf ]; then \
		install -D -m 755 sangoma_mgd.conf.sample.ss7 $(INSTALLPREFIX)/etc/sangoma_mgd.conf; \
	fi
	install -D -m 755 smg_ctrl_ss7 $(INSTALLPREFIX)/usr/sbin/smg_ctrl
	install -D -m 755 scripts/init.d/smgss7_init_ctrl $(INSTALLPREFIX)/etc/init.d/smgss7_init_ctrl
	install -D -m 755 scripts/init.d/smgss7_init_ctrl $(INSTALLPREFIX)/usr/sbin/smgss7_init_ctrl

endif	
	@echo "sangoma_mgd Installed"

old_cleanup:
	./scripts/old_cleanup.sh


install_all: all install_smg 

uninstall: 
	/bin/rm $(INSTALLPREFIX)/usr/sbin/sangoma_mgd $(INSTALLPREFIX)/etc/sangoma_mgd.conf



