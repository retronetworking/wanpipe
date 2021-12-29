# ============================================================================
# Makefile	WAN Echo Canceler Chip configurator.  Make Script.
#
# Copyright	(c) 1995-1997 Sangoma Technologies Inc.  All Rights Reserved.
# ----------------------------------------------------------------------------
# Augc 25, 2005		Alex Feldman	Initial version.
# ============================================================================

####### DEFINES ##############################################################

# Build Options.
PWD = $(shell pwd)    
PROD	= wan_ec_setup
VERSION	= 1.0
DEBUG	= 2
YACC	= yacc

# Project File Paths.
TMPDIR		= tmp
OUTDIR		= mod/
SYSINC		= /lib/modules/$(shell uname -r)/build/include 
VPATH		= $(LOCINC):$(TMPDIR)
ARCH    	= $(shell uname -m)
DIR_EC_APILIB	= ../wanec_apilib

PRODS=wan_ec_client
SRC_CLIENT=wan_ecmain.c y.tab.c wan_ec_argl.c

SRC_EC_APILIB = $(DIR_EC_APILIB)/wanec_api.c $(DIR_EC_APILIB)/wanec_api_lib.c
INC_EC_APILIB = -I../../patches/kdrivers/wanec -I../../patches/kdrivers/include 
INC_EC_APILIB += -I../../patches/kdrivers/wanec/oct6100_api/include 
INC_EC_APILIB += -I$(DIR_EC_APILIB)

CFLAGS	= -Wall -O2 -D__LINUX__ -DCONFIG_PRODUCT_WANPIPE_TDMV_EC -D_DEBUG_=$(DEBUG) -D_GNUC_
CFLAGS	+= -I$(SYSINC) $(INC_EC_APILIB) $(EXTRA_FLAGS)

#$(WANEC_DIR) -I$(WANEC_DIR)/oct6100_api/include -I$(WANEC_DIR)/oct6100_api/octdeviceapi/oct6100api/

####### RULES ################################################################

all:	$(PRODS) 
	@echo "Ok."

wan_ec_client:	$(SRC_CLIENT) $(SRC_EC_APILIB)
	$(CC) $(CFLAGS) -o $@ $^ 

y.tab.c:
	$(YACC) -d wan_ec_arg.y

wan_ec_argl.c:
	$(LEX)	-owan_ec_argl.c wan_ec_arg.l

install:
	@echo "Installing Wanpipe EC API in $(WAN_VIRTUAL)/usr/sbin"
	install $(PRODS) $(WAN_VIRTUAL)/usr/sbin/

uninstall:
	@echo "Un-installing Wanpipe EC API from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/$(PRODS)

clean:
	rm -f $(PRODS)
	rm -f y.tab.c
	rm -f y.tab.h
	rm -f wan_ec_argl.c
