#
# Makefile for the Linux network (wan) device drivers.
#
# 3 Aug 2000, Christoph Hellwig <hch@infradead.org>
# Rewritten to use lists instead of if-statements.
#

FRDIR=../fr
SPPPDIR=../sppp
ATMDIR=../atm
LIP_KATMDIR=../lip_katm
LIP_ATMDIR=../lip_atm
COMMON=../wanpipe
ROUTER=../router
LIP_HDLC=../lip_hdlc

CONFIG_WANPIPE_FR=y
CONFIG_WANPIPE_SPPP=y
CONFIG_WANPIPE_ATM=n 
CONFIG_WANPIPE_LIP_KATM=y 
CONFIG_WANPIPE_LIP_ATM=y 
CONFIG_WANPIPE_LIP_TTY=y 
CONFIG_WANPIPE_LIP_HDLC=y 

EXTRA_CFLAGS=$(EXTRA_FLAGS)
EXTRA_CFLAGS+=-DWANLIP_DRIVER -DWPLIP_TTY_SUPPORT 

wanpipe_lip-y	:= wanpipe_lip_iface.o wanpipe_lip_sub.o 
wanpipe_lip-y 	+= wanpipe_lip_netdev.o wanpipe_lip_prot.o 
wanpipe_lip-y 	+= wanpipe_lip_bh.o wanpipe_lip_ipx.o 
wanpipe_lip-y 	+= $(COMMON)/wanpipe_abstr.o $(ROUTER)/wanpipe_linux_iface.o  


ifeq "${CONFIG_WANPIPE_LIP_HDLC}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_LIP_HDLC 
wanpipe_lip-y			+= wanpipe_hdlc_iface.o
wanpipe_lip-y			+= wanpipe_hdlc_eng.o
endif 


ifeq "${CONFIG_WANPIPE_LIP_TTY}" "y"
PRODUCT_DEFINES			+= -DWPLIP_TTY_SUPPORT 
wanpipe_lip-y			+= wanpipe_lip_tty.o
endif 

ifeq "${CONFIG_WANPIPE_FR}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_FR
wanpipe_lip-y			+= $(FRDIR)/wanpipe_fr_iface.o   $(FRDIR)/wanpipe_fr_snmp.o
EXTRA_CFLAGS 			+= -I$(FRDIR) -I$(FRDIR)/include
endif

ifeq "${CONFIG_WANPIPE_SPPP}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_CHDLC -DCONFIG_PRODUCT_WANPIPE_PPP
wanpipe_lip-y			+= $(SPPPDIR)/wanpipe_sppp_iface.o $(SPPPDIR)/wanpipe_sppp_in.o 
wanpipe_lip-y			+= $(SPPPDIR)/wanpipe_sppp_auth.o $(SPPPDIR)/wanpipe_sppp_sub.o
EXTRA_CFLAGS 			+= -I$(SPPPDIR) -I$(SPPPDIR)/include -I/common/sppp/include
endif

#ifeq "${CONFIG_WANPIPE_LIP_KATM}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_LIP_KATM
wanpipe_lip-y			+= $(LIP_KATMDIR)/wanpipe_katm_iface.o 
wanpipe_lip-y			+= $(LIP_KATMDIR)/wanpipe_katm_sub.o 
EXTRA_CFLAGS 			+= -I$(LIP_KATMDIR) -I$(LIP_KATMDIR)/include -I/common/lip_katm/include
#endif        

#ifeq "${CONFIG_WANPIPE_LIP_ATM}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_LIP_ATM
wanpipe_lip-y			+= $(LIP_ATMDIR)/wanpipe_lip_atm_iface.o
EXTRA_CFLAGS 			+= -I$(LIP_ATMDIR) -I$(LIP_ATMDIR)/include -I/common/lip_atm/include
#endif   

ifeq "${CONFIG_WANPIPE_ATM}" "y"
PRODUCT_DEFINES			+= -DCONFIG_PRODUCT_WANPIPE_ATM
wanpipe_lip-y			+= $(ATMDIR)/wanpipe_atm.o
EXTRA_CFLAGS 			+= -I$(ATMDIR) -I$(ATMDIR)/include
endif

EXTRA_CFLAGS 			+= $(PRODUCT_DEFINES) 


wanpipe_lip-objs		:= $(wanpipe_lip-y)

obj-m += wanpipe_lip.o    

