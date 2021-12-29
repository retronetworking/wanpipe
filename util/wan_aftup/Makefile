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

PROD=wan_aftup

WAN_VIRTUAL=


# Tools options.
SRCS	= wan_aftup.c wan_aft_prg.c wan_aft_flash.c wan_aft_flash_shark.c wan_aft_flash_shark_ds.c wan_aft_flash_a600.c
SRCS	+= wan_pcie_ctrl.c wan_pcie_ctrl_tundra.c wan_pcie_ctrl_plx.c
CFLAGS  = -Wall -O2 -D$(OS_TYPE) -D_DEBUG_=$(DEBUG) -lstdc++
CFLAGS += $(EXTRA_FLAGS) -I../../patches/kdrivers/include -I/usr/include/wanpipe


####### RULES ################################################################

all:	$(PROD) 
	@echo "Ok."

wan_aftup:	$(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

install:
	@echo "Installing Wanpipe Firmware update utility in $(WAN_VIRTUAL)/usr/sbin"
	install -D $(PROD)	  $(WAN_VIRTUAL)/usr/sbin/$(PROD)

install:
	@echo "Installing WAN_AFTUP in $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup"
	install -d $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup/scripts
	install -m755 wan_aftup $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup/wan_aftup
	install -m755 update_aft_firm.sh $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup/update_aft_firm.sh
	install -m755 scripts/load.sh $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup/scripts/load.sh
	install -m755 scripts/unload.sh $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup/scripts/unload.sh

uninstall:
	@echo "Uninstalling WAN_AFTUP in $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup"
	rm -rf $(WAN_VIRTUAL)/etc/wanpipe/util/wan_aftup

clean:
	rm -f $(PROD)
