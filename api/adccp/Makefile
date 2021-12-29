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

# Tools options.
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_ -I../lib -I$(SYSINC) -I$(APIINC)

####### RULES ################################################################

all:	adccp_api  
	@echo "Ok."

adccp_api:	adccp_api.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^ 

clean:
	rm -f adccp_api
