# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

DIR_EC_APILIB	= ../wanec_apilib

WANECDIR=../../patches/kdrivers/wanec
ifeq (,$(wildcard $(WANECDIR)))
	WANECDIR=/common/wanec
endif

ifndef SYSINC
	SYSINC=/usr/include/wanpipe
endif

WAN_VIRTUAL=

# Tools options.
SRCS=wanconfig.c wanconfig_hwec.c $(DIR_EC_APILIB)/wanec_api.c $(DIR_EC_APILIB)/wanec_api_lib.c
CFLAGS = -Wall -O2 -D$(OS_TYPE) -I$(SYSINC) -D_DEBUG_=$(DEBUG)
CFLAGS += -DCONFIG_PRODUCT_WANPIPE_AFT_BRI -DCONFIG_PRODUCT_WANPIPE_USB
CFLAGS += -I../../patches/kdrivers/include -I$(SYSINC)
CFLAGS += -I$(DIR_EC_APILIB)
CFLAGS += -I$(WANECDIR)/oct6100_api/include
CFLAGS += -I$(WANECDIR) 
CFLAGS += $(EXTRA_FLAGS)

CFLAGS += -DWAN_HWEC

####### RULES ################################################################

all: wanconfig
	@echo "Ok."

wanconfig:	$(SRCS) wanconfig.h
	$(CC) $(CFLAGS) -o $@ $^

install:
	@echo "Installing Wanpipe utilites in $(WAN_VIRTUAL)/usr/sbin"
	install -D wanconfig  $(WAN_VIRTUAL)/usr/sbin/wanconfig

uninstall:
	@echo "Un-installing Wanpipe utilites from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/wanconfig

clean:
	rm -f wanconfig 
