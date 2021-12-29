# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

SYSINC=$(shell if [ -f ../../.sysinclude ]; then cat ../../.sysinclude;  else echo ""; fi)
ifeq "$(SYSINC)" "" 
SYSINC=/lib/modules/$(shell uname -r)/build/include
endif
WAN_VIRTUAL=

# Tools options.
CFLAGS  = -Wall -O2 -D$(OS_TYPE) -I$(SYSINC) -I../../patches/kdrivers/include -D_DEBUG_=$(DEBUG)

CFLAGS += -DWAN_HWEC

####### RULES ################################################################

all: wanconfig
	@echo "Ok."

wanconfig:	wanconfig.c 
	$(CC) $(CFLAGS) -o $@ $<

install:
	@echo "Installing Wanpipe utilites in $(WAN_VIRTUAL)/usr/sbin"
	install -D wanconfig  $(WAN_VIRTUAL)/usr/sbin/wanconfig

uninstall:
	@echo "Un-installing Wanpipe utilites from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/wanconfig

clean:
	rm -f wanconfig 
