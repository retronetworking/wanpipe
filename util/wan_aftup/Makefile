# ============================================================================
# Makefile	Multiprotocol WAN Router for Linux.  Make Script.
#
# Copyright	(c) 1995-1997 Sangoma Technologies Inc.  All Rights Reserved.
# ----------------------------------------------------------------------------
# Mar 27  2000  Nenad Corbic	Version 2.0.5 to 2.1.2
# Jan 07, 1999	Jaspreet Singh	Version 2.0.4
# Aug 25, 1998	Jaspreet Singh  Version 2.0.3
# Nov 06, 1997	Jaspreet Singh	Version 2.0.0
# Jul 28, 1997	Jaspreet Singh  Version 1.0.5
# Jul 10, 1997  Jaspreet Singh	Version 1.0.4
# June 3, 1997	Jaspreet Singh	Version 1.0.3	
# Jan 15, 1997	Gene Kozin	Version 1.0.1.
# Dec 31, 1996	Gene Kozin	Initial version.
# ============================================================================

####### DEFINES ##############################################################

# Build Options.
#
OS_TYPE	= __LINUX__
DEBUG	= 2


SYSINC=$(shell if [ -f ../../.sysinclude ]; then cat ../../.sysinclude;  else echo ""; fi)

ifeq "$(SYSINC)" "" 
# Project file paths.
SYSINC=/lib/modules/$(shell uname -r)/build/include
endif

WAN_VIRTUAL=


# Tools options.
CFLAGS  = -Wall -O2 -D$(OS_TYPE) -I$(SYSINC) -D_DEBUG_=$(DEBUG)


####### RULES ################################################################

all:	wan_aftup 
	@echo "Ok."

wan_aftup:	wan_aftup.c wan_aft_prg.c wan_aft_flash.c wan_aft_flash_shark.c wan_aft_flash_shark_ds.c wan_plxctrl.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f wan_aftup
