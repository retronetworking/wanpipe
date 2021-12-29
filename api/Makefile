# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

SYSINC=$(shell if [ -f ../.sysinclude ]; then cat ../.sysinclude;  else echo ""; fi)
ifeq "$(SYSINC)" "" 
SYSINC=/lib/modules/$(shell uname -r)/build/include
endif
APIINC=/etc/wanpipe/api/include

####### RULES ################################################################

all:	
	make -C x25 all SYSINC=$(SYSINC) APIINC=$(APIINC)
	make -C chdlc all SYSINC=$(SYSINC) APIINC=$(APIINC)
	make -C fr all SYSINC=$(SYSINC) APIINC=$(APIINC)
	make -C bitstrm all SYSINC=$(SYSINC) APIINC=$(APIINC)
	make -C bisync all SYSINC=$(SYSINC) APIINC=$(APIINC)
#	make -C ss7 all SYSINC=$(SYSINC) APIINC=$(APIINC)

clean:
	make -C x25 clean 
	make -C chdlc clean 
	make -C fr clean 
	make -C bitstrm clean
	make -C bisync clean
	make -C tdm_api clean
	make -C lib/hdlc clean
#	make -C ss7 clean
