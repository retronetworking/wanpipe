# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-1998 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

# Project file paths.
SYSINC	= /usr/include/wanpipe
APIINC  = /usr/include/wanpipe

# Tools options.
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_ -I../lib -I$(SYSINC) -I$(APIINC)

####### RULES ################################################################

all:	chdlc_api \
	chdlc_rts_cts \
	chdlc_integrity \
	chdlc_modem_cmd \
	chdlc_echo
	@echo "Ok."

chdlc_api:	chdlc_api.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

chdlc_echo:	chdlc_echo.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^


chdlc_rts_cts:	chdlc_rts_cts.c
	$(CC) $(CFLAGS) -o $@ $<

chdlc_integrity:	chdlc_integrity.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

chdlc_modem_cmd:	chdlc_modem_cmd.c ../lib/lib_api.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f chdlc_api
	rm -f chdlc_rts_cts
	rm -f chdlc_integrity
