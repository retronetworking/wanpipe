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
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_ -I../lib/hdlc -I../lib -I$(SYSINC)

TARGETS = aft_tdm_voice_api
TARGETS += aft_tdm_voice_api_rbs
TARGETS += aft_tdm_hdlc_test

####### RULES ################################################################

all:	$(TARGETS)  
	@echo "Ok."

aft_tdm_voice_api: aft_tdm_voice_api.c ../lib/lib_api.c 
	$(CC) $(CFLAGS) -o $@ $^ -lsangoma -lm

aft_tdm_voice_api_rbs: aft_tdm_voice_api_rbs.c ../lib/lib_api.c 
	$(CC) $(CFLAGS) -o $@ $^ -lsangoma -lm

aft_tdm_hdlc_test:	aft_tdm_hdlc_test.c ../lib/hdlc/wanpipe_hdlc.c
	$(CC) $(CFLAGS) -o $@ $^ -lsangoma -lm  

clean:
	rm -f $(TARGETS)
