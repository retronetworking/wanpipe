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

DIR_EC_APILIB=../wanec_apilib
SRC_EC_APILIB=$(DIR_EC_APILIB)/wanec_api.c $(DIR_EC_APILIB)/wanec_api_lib.c 
INC_EC_APILIB=-I/usr/include/wanpipe/oct6100_api -I$(DIR_EC_APILIB)

# Tools options.
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -DCONFIG_PRODUCT_WANPIPE_TDMV_EC -D_GNUC_ -I../lib -I$(SYSINC)
CFLAGS += -I/usr/include/wanpipe

TARGETS=aft_api
TARGETS+= aft_api_events
TARGETS+= aft_integrity
TARGETS+= aft_echo
#TARGETS+= aft_api_ss7
#TARGETS+= aft_api_check
#TARGETS+= aft_tdm_api

####### RULES ################################################################

all:	$(TARGETS)  
	@echo "Ok."

aft_api:     	aft_api.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_api_events: 	aft_api_events.c $(SRC_EC_APILIB) ../lib/lib_api.c
	$(CC) $(CFLAGS) $(INC_EC_APILIB) -o $@ $^

aft_api_check: 	aft_api_check.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_tdm_api: 	aft_tdm_api.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_api_ss7: 	aft_api_ss7.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_integrity: 	aft_integrity.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

aft_echo: 	aft_echo.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
