# ============================================================================
# Makefile	Sangoma PLX EEPROM control.
#
# Copyright	(c) 2006 Sangoma Technologies Inc.  All Rights Reserved.
# ----------------------------------------------------------------------------
# Nov 3, 2006i	Alex Feldman	Initial version.
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

PRODS=wan_plxup

# Tools options.
CFLAGS  = -Wall -O2 -D$(OS_TYPE) -I$(SYSINC) -D_DEBUG_=$(DEBUG)


####### RULES ################################################################

all:	$(PRODS) 
	@echo "Ok."

wan_plxup:	wan_plxup.c wan_plxctrl.c
	$(CC) $(CFLAGS) -o $@ $^

install:
	@echo "Installing Wanpipe PLX in $(WAN_VIRTUAL)/usr/sbin"
	install $(PRODS) $(WAN_VIRTUAL)/usr/sbin/

uninstall:
	@echo "Un-installing Wanpipe PLX from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/$(PRODS)
                               	
clean:
	rm -f wan_plxup
