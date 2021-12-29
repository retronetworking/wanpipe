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
SYSINC	= /usr/src/linux/include
VPATH	= $(SYSINC)
APIINC  = ./

# Tools options.
CFLAGS	= -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -D_GNUC_ -I$(SYSINC) -I$(APIINC)

####### RULES ################################################################

all:	sdlc_api
	@echo "Ok."

sdlc_api:	sdlc_api.c
	$(CC) $(CFLAGS) -o $@ $^


clean:
	rm -f sdlc_api
