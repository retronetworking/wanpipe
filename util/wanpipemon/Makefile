# ============================================================================
# Makefile	Make script for building Linux WAN router utilities.
# ----------------------------------------------------------------------------
# Copyright (c) 1995-2000 Sangoma Technologies Inc.  All Rights Reserved.
# ============================================================================

####### MACROS ###############################################################

# Build options.
OS_TYPE	= __LINUX__
DEBUG	= 2

PROD=wanpipemon

SYSINC=$(shell if [ -d /lib/modules/$(uname -r)/build/include ]; then echo "/lib/modules/$(uname -r)/build/include" else echo "/usr/src/linux/include"; fi)
SYSINC=$(shell echo "/lib/modules/$(uname -r)/build/include")

WAN_VIRTUAL=
LXDIR=../lxdialog
ENABLE_GUI=YES
ENABLE_WANPIPEMON_ZAP=
PROTOCOL_DEFS=
ASTBROOT=/usr
#-DCONFIG_PRODUCT_WANPIPE_ADSL

CC = gcc 
CPP = gcc -E
CFLAGS = -Wall -Wstrict-prototypes -I$(SYSINC) -D$(OS_TYPE) -I../ft1 -O2 -fomit-frame-pointer -DLOCALE -D_DEBUG_=$(DEBUG) $(HOSTCFLAGS)
LDFLAGS = -s -L .
WAN_VIRTUAL=


HOSTCFLAGS=-I$(SYSINC)
#ifeq (/usr/include/ncurses/ncurses.h, $(wildcard /usr/include/ncurses/ncurses.h))
#        HOSTCFLAGS += -I/usr/include/ncurses
#else
#ifeq (/usr/include/ncurses/curses.h, $(wildcard /usr/include/ncurses/curses.h))
#        HOSTCFLAGS += -I/usr/include/ncurses
#else
#endif
#endif

HOSTCFLAGS += -I$(ASTBROOT)/include     

# Tools options.

ifeq "${ENABLE_GUI}" "NO"
LX_OBJ =
GUI_OBJ =
LDLIBS =
else
LX_OBJ = $(LXDIR)/checklist.o $(LXDIR)/menubox.o $(LXDIR)/inputbox.o $(LXDIR)/util.o 
GUI_OBJ = wangui.o
LDLIBS = -lncurses

CFLAGS += -DWANPIPEMON_GUI -DLOCALE 

ifeq "${ENABLE_WANPIPEMON_ZAP}" "YES"
CFLAGS += -DWANPIPEMON_ZAP
LDLIBS += -lstdc++
endif

CFLAGS += -DCURSES_LOC="<ncurses.h>"       

#ifeq (/usr/include/ncurses/ncurses.h, $(wildcard /usr/include/ncurses/ncurses.h))
#        CFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses.h>"
#else
#ifeq (/usr/include/ncurses/curses.h, $(wildcard /usr/include/ncurses/curses.h))
#        CFLAGS += -I/usr/include/ncurses -DCURSES_LOC="<ncurses/curses.h>"
#else
#ifeq (/usr/include/ncurses.h, $(wildcard /usr/include/ncurses.h))
#        CFLAGS += -DCURSES_LOC="<ncurses.h>"
#else
#	CFLAGS += -DCURSES_LOC="<curses.h>"
#endif
#endif
#endif

endif

OBJS= wanpipemon.o fpipemon.o cpipemon.o ppipemon.o xpipemon.o atmpipemon.o aftpipemon.o dslpipemon.o prot_trace.o ss7pipemon.o bpipemon.o fe_lib.o xml_lib.o ../ft1/unixio.o
OBJS+=$(GUI_OBJ)

ifeq "${ENABLE_WANPIPEMON_ZAP}" "YES"
OBJS+=zapmon.o
endif

SRCS = $(OBJS:.o=.c)

OBJS+=$(LX_OBJ)


####### RULES ################################################################

all:    ncurses \
	$(PROD) 
	@echo "Ok. Using INCLUDE $(SYSINC)"

$(PROD):  $(OBJS) 

ncurses:
	@x=`find $(ASTBROOT)/lib/ /lib/ /usr/lib/ /usr/lib64/ /usr/local/lib/ -maxdepth 1 -name 'libncurses.*' 2> /dev/null` ;\
	if [ ! "$$x" ]; then \
		echo -e "\007" ;\
		echo ">> Unable to find the Ncurses libraries." ;\
		echo ">>" ;\
		echo ">> You must have Ncurses installed in order" ;\
		echo ">> to use 'make menuconfig'" ;\
		echo ;\
		exit 1 ;\
	fi

nenad:
	@echo "Ok. Using INCLUDE $(SYSINC)"  
	
install:
	@echo "Installing Wanpipe utilites in $(WAN_VIRTUAL)/usr/sbin"
	install -D $(PROD)	  $(WAN_VIRTUAL)/usr/sbin/$(PROD)

uninstall:
	@echo "Un-installing Wanpipe utilites from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/$(PROD)
clean:
	rm -f *.o
	rm -f $(PROD)
