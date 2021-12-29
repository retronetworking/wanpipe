# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-1998 Sangoma Technologies Inc.  All Rights Reserved.
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
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_ -I../lib -I$(SYSINC)

TARGETS=aft_api
TARGETS+= aft_api_ss7
TARGETS+= aft_api_check

####### RULES ################################################################

all:	$(TARGETS)  
	@echo "Ok."

aft_api:     	aft_api.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_api_check: 	aft_api_check.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_api_ss7: 	aft_api_ss7.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^



clean:
	rm -f $(TARGETS)
