# ============================================================================
# Makefile	Make script for building SS7 MTP2 API
# ----------------------------------------------------------------------------
# Author:	Nenad Corbic <ncorbic@sangoma.com>
# Copyright (c) 2008 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

SYSINC=$(shell if [ -f ../../.sysinclude ]; then cat ../../.sysinclude;  else echo ""; fi)
ifeq "$(SYSINC)" "" 
SYSINC=/usr/src/linux/include
endif
VPATH	= $(SYSINC)

# Tools options.
CFLAGS	= -Wall -O0  -g -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_  -I$(SYSINC)

TARGETS=mtp2api

####### RULES ################################################################

all:	$(TARGETS)  
	@echo "Ok."

mtp2api:     	mtp2api.c libxmtp2.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
	rm -f *.o
	rm -f *.*~
	rm -f diff
