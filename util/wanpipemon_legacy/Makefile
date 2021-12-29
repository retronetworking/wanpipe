# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

PROD=wanpipemon_legacy
# Project file paths.
SYSINC=/lib/modules/$(shell uname -r)/build/include
WAN_VIRTUAL=
LXDIR=../lxdialog
ENABLE_GUI=YES
PROTOCOL_DEFS=
#-DCONFIG_PRODUCT_WANPIPE_ADSL

CC = gcc 
CPP = gcc -E
CFLAGS = -Wall -Wstrict-prototypes -I$(SYSINC) -D$(OS_TYPE) -I../ft1 -O2 -fomit-frame-pointer -DLOCALE -D_DEBUG_=$(DEBUG) -DCURSES_LOC="<ncurses.h>"
WAN_VIRTUAL=

CFLAGS += $(EXTRA_FLAGS)

ifeq "${ENABLE_GUI}" "NO"
LX_OBJ =
GUI_OBJ =
LDLIBS =
else
LX_OBJ = $(LXDIR)/checklist.o $(LXDIR)/menubox.o $(LXDIR)/inputbox.o $(LXDIR)/util.o 
GUI_OBJ = wangui.o
LDLIBS = -lncurses

CFLAGS += -DWANPIPEMON_GUI -DLOCALE  
endif

OBJS= wanpipemon_legacy.o fpipemon.o cpipemon.o ppipemon.o xpipemon.o atmpipemon.o aftpipemon.o dslpipemon.o prot_trace.o ss7pipemon.o bpipemon.o fe_lib.o xml_lib.o ../ft1/unixio.o 
OBJS+=$(GUI_OBJ)

SRCS = $(OBJS:.o=.c)

OBJS+=$(LX_OBJ)

####### RULES ################################################################

all:    $(PROD) 
	@echo "Ok."

$(PROD):  $(OBJS) 

install:
	@echo "Installing Wanpipe utilites in $(WAN_VIRTUAL)/usr/sbin"
	install -D $(PROD)	  $(WAN_VIRTUAL)/usr/sbin/$(PROD)

uninstall:
	@echo "Un-installing Wanpipe utilites from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/$(PROD)
clean:
	rm -f *.o
	rm -f $(PROD)
