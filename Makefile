#
# Makefile	WANPIPE WAN Router Installation/Removal Makefile
#
# Copyright	(c) 2007, Sangoma Technologies Inc.
#
#		This program is free software; you can redistribute it and/or
#		modify it under the terms of the GNU General Public License
#		as published by the Free Software Foundation; either version
#		2 of the License, or (at your option) any later version.
# ----------------------------------------------------------------------------   
# Author:  Nenad Corbic <ncorbic@sangoma.com> 
#

PWD=$(shell pwd)
KBUILD_VERBOSE=0

#Default zaptel directory to be overwritten by user
ifndef ZAPDIR
	ZAPDIR=/usr/src/zaptel
endif

#Kernel version and location
ifndef KVER
	KVER=$(shell uname -r)
endif
ifndef KMOD
	KMOD=/lib/modules/$(KVER)
endif
ifndef KDIR
	KDIR=$(KMOD)/build
endif
ifndef KINSTDIR
	KINSTDIR=$(KMOD)/kernel
endif

ifndef ARCH
	ARCH=$(shell uname -m)
endif
ifndef ASTDIR
    	ASTDIR=/usr/src/asterisk
endif


INSTALLPREFIX=

#Local wanpipe includes
WINCLUDE=patches/kdrivers/include
HWECINC=patches/kdrivers/wanec/oct6100_api
KMODDIR=patches/kdrivers

#Location of wanpipe source in release
WAN_DIR=$(PWD)/$(KMODDIR)/src/net
WANEC_DIR=$(PWD)/$(KMODDIR)/wanec
MODTYPE=ko

#Setup include path and extra cflags
EXTRA_CFLAGS := -I$(PWD)/$(WINCLUDE) -I$(PWD)/$(WINCLUDE)/annexg -I$(PWD)/patches/kdrivers/wanec -D__LINUX__
EXTRA_CFLAGS += -I$(WANEC_DIR) -I$(WANEC_DIR)/oct6100_api -I$(WANEC_DIR)/oct6100_api/include

#Setup utility extra flags and include path
EXTRA_UTIL_FLAGS = -I$(PWD)/$(WINCLUDE) -I$(KDIR)/include/ -I$(INSTALLPREFIX)/include -I$(INSTALLPREFIX)/usr/include 
EXTRA_UTIL_FLAGS += -I$(PWD)/patches/kdrivers/wanec -I$(PWD)/patches/kdrivers/wanec/oct6100_api/include

ENABLE_WANPIPEMON_ZAP=NO
ZAPHDLC_PRIV=/etc/wanpipe/.zaphdlc



#Check if zaptel exists
ifneq (,$(wildcard $(ZAPDIR)/zaptel.h))
	ZAPDIR_PRIV=$(ZAPDIR) 
	ENABLE_WANPIPEMON_ZAP=YES
	EXTRA_CFLGS+= -DSTANDALONE_ZAPATA -DBUILDING_TONEZONE
	ZAP_OPTS= --zaptel-path=$(ZAPDIR) 
	ZAP_PROT=TDM
	PROTS=DEF
else
	ifneq (,$(wildcard $(ZAPDIR)/kernel/zaptel.h))
		ZAPDIR=/usr/src/zaptel/kernel
		ZAPDIR_PRIV=$(ZAPDIR) 
		ENABLE_WANPIPEMON_ZAP=YES
		EXTRA_CFLGS+= -DSTANDALONE_ZAPATA -DBUILDING_TONEZONE
		ZAP_OPTS= --zaptel-path=$(ZAPDIR) 
		ZAP_PROT=TDM
		PROTS=DEF-TDM
	else
		ZAP_OPTS=
		ZAP_PROT=
		ZAPDIR_PRIV=
		ENABLE_WANPIPEMON_ZAP=NO
		PROTS=DEF
	endif
endif  

EXTRA_CFLAGS += -I$(KDIR)/include/linux -I$(ZAPDIR)

RM      = @rm -rf
JUNK	= *~ *.bak DEADJOE


# First pass, kernel Makefile reads module objects
ifneq ($(KERNELRELEASE),)
obj-m := sdladrv.o wanrouter.o wanpipe.o wanpipe_syncppp.o wanec.o 

# Second pass, the actual build.
else

#This will check for zaptel, kenrel source and build utilites and kernel modules
#within local directory structure
all:   _checkzap _checksrc all_bin_kmod all_util 	

all_src:   _checkzap _checksrc all_kmod all_util

#Build only kernel modules
all_kmod:  _checkzap _checksrc _cleanoldwanpipe _check_kver
	$(MAKE) KBUILD_VERBOSE=$(KBUILD_VERBOSE) -C $(KDIR) SUBDIRS=$(WAN_DIR) EXTRA_FLAGS="$(EXTRA_CFLAGS) $(shell cat ./patches/kfeatures)" ZAPDIR=$(ZAPDIR_PRIV) ZAPHDLC=$(ZAPHDLC_PRIV) HOMEDIR=$(PWD) modules  

all_bin_kmod:  _checkzap _checksrc _cleanoldwanpipe _check_kver
	@if [ -e  $(PWD)/ast_build_dir ]; then \
		rm -rf $(PWD)/ast_build_dir; \
	fi
	@mkdir -p $(PWD)/ast_build_dir
	./Setup drivers --builddir=$(PWD)/ast_build_dir --with-linux=$(KDIR) $(ZAP_OPTS) --usr-cc=$(CC) --protocol=$(PROTS) --no-zaptel-compile --noautostart --arch=$(ARCH) --silent
	@eval "./patches/copy_modules.sh $(PWD)/ast_build_dir $(WAN_DIR)"


#Clean utilites and kernel modules
clean:  clean_util _cleanoldwanpipe
	$(MAKE) -C $(KDIR) SUBDIRS=$(WAN_DIR) clean
	@find patches/kdrivers -name '.*.cmd' | xargs rm -f
	@find . -name 'Module.symver*' | xargs rm -f

                                    
#Clean old wanpipe headers from linux include
_cleanoldwanpipe: _checksrc
	@eval "./patches/build_links.sh"
	@eval "./patches/clean_old_wanpipe.sh $(WINCLUDE) $(KDIR)/include/linux"
	

#Check for linux headers
_checksrc: 
	@if [ ! -e $(KDIR) ]; then \
		echo "   Error linux headers/source not found: $(KDIR) !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/.config ]; then \
		echo "   Error linux headers/source not configured: missing $(KDIR)/.config !"; \
		echo ; \
		exit 1; \
	fi 
	@if [ ! -e $(KDIR)/include ]; then \
		echo "   Error linux headers/source incomplete: missing $(KDIR)/include dir !"; \
		echo ; \
		exit 1; \
	fi

_check_kver:
	@eval "./patches/kern_i_private_check.sh $(KDIR)"
	@echo > ./patches/kfeatures; 
	@if [ -e ./patches/i_private_found ]; then \
		echo "-DWANPIPE_USE_I_PRIVATE " >> ./patches/kfeatures; \
	fi

#Check for zaptel
_checkzap:
	@echo
	@echo " +--------- Wanpipe Build Info --------------+"  
	@echo 
	@if [ ! -e $(ZAPDIR)/zaptel.h ]; then \
		echo " Compiling Wanpipe without ZAPTEL Support!"; \
		ZAPDIR_PRIV=; \
		ENABLE_WANPIPEMON_ZAP=NO; \
	else \
		echo "   Compiling Wanpipe with ZAPTEL Support!"; \
		echo "      Zaptel Dir: $(ZAPDIR)"; \
		echo; \
		eval "$(PWD)/patches/sangoma-zaptel-patch.sh $(ZAPDIR)"; \
		ZAPDIR_PRIV=$(ZAPDIR); \
		ENABLE_WANPIPEMON_ZAP=YES; \
		cp -f $(ZAPDIR)/Module.symvers $(WAN_DIR)/; \
		echo ; \
		echo "Please recompile and reinstall ZAPTEL after installation"; \
	fi
	@echo 
	@echo " +-------------------------------------------+" 
	@echo 
	@sleep 2; 

#Install all utilities etc and modules
install: install_util install_etc install_kmod install_inc

#Install kernel modules only
install_kmod:
	install -m 644 -D $(WAN_DIR)/wanrouter.${MODTYPE}  	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanrouter.${MODTYPE} 
	install -m 644 -D $(WAN_DIR)/af_wanpipe.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/af_wanpipe.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/wanec.${MODTYPE} 		$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanec.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/wan_aften.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wan_aften.${MODTYPE}   
	install -m 644 -D $(WAN_DIR)/sdladrv.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/sdladrv.${MODTYPE}
	install -m 644 -D $(WAN_DIR)/wanpipe.${MODTYPE} 	$(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe.${MODTYPE}            
	@rm -f $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}
	@if [ -f  $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} ]; then \
		echo "install -m 644 -D $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}"; \
		install -m 644 -D $(WAN_DIR)/wanpipe_syncppp.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/drivers/net/wan/wanpipe_syncppp.${MODTYPE}; \
	fi
	@rm -f $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}; 
	@if [ -f  $(WAN_DIR)/wanpipe_lip.${MODTYPE} ]; then \
		echo "install -m 644 -D $(WAN_DIR)/wanpipe_lip.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}"; \
		install -m 644 -D $(WAN_DIR)/wanpipe_lip.${MODTYPE} $(INSTALLPREFIX)/$(KINSTDIR)/net/wanrouter/wanpipe_lip.${MODTYPE}; \
	fi
	@eval "./patches/rundepmod.sh"	
	
endif

#Compile utilities only
all_util:  install_inc
	$(MAKE) -C util all EXTRA_FLAGS="$(EXTRA_UTIL_FLAGS)" SYSINC="$(PWD)/$(WINCLUDE) -I $(PWD)/api/libsangoma/include" CC=$(CC) \
	PREFIX=$(INSTALLPREFIX) HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" ARCH=$(ARCH) 
	$(MAKE) -C util all_wancfg EXTRA_FLAGS="$(EXTRA_UTIL_FLAGS)" SYSINC="$(PWD)/$(WINCLUDE) -I$(PWD)/api/libsangoma/include" CC=$(CC) \
	PREFIX=$(INSTALLPREFIX) HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" HOSTCFLAGS="$(EXTRA_UTIL_FLAGS)" ARCH=$(ARCH)

#Clean utilities only
clean_util:	
	$(MAKE) -C util clean SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)

#Install utilities only
install_util:
	$(MAKE) -C util install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)  

install_smgbri:
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ install SYSINC=$(PWD)/$(WINCLUDE) CC=$(CC) PREFIX=$(INSTALLPREFIX)
	install -D -m 755 ssmg/sangoma_bri/smg_ctrl $(INSTALLPREFIX)/usr/sbin/smg_ctrl
	install -D -m 755 ssmg/sangoma_bri/sangoma_brid $(INSTALLPREFIX)/usr/sbin/sangoma_brid
	$(MAKE) -C ssmg/libsangoma.trunk/ install DESTDIR=$(INSTALLPREFIX)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone install DESTDIR=$(INSTALLPREFIX)


#Install etc files
install_etc:
	@if [ ! -e $(INSTALLPREFIX)/etc/wanpipe ]; then \
	      	mkdir -p $(INSTALLPREFIX)/etc/wanpipe; \
	fi
	@if [ ! -e $(INSTALLPREFIX)/etc/wanpipe/wanrouter.rc ]; then \
		install -D -m 644 samples/wanrouter.rc $(INSTALLPREFIX)/etc/wanpipe/wanrouter.rc; \
	fi
	@if [ ! -e  $(INSTALLPREFIX)/etc/wanpipe/lib ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/lib; \
	fi
	@\cp -f util/wancfg_legacy/lib/* $(INSTALLPREFIX)/etc/wanpipe/lib/
	@\cp -rf firmware  $(INSTALLPREFIX)/etc/wanpipe/
	@if [ ! -f $(INSTALLPREFIX)/etc/wanpipe/interfaces ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/interfaces; \
	fi    
	@\cp -rf samples $(INSTALLPREFIX)/etc/wanpipe
	@if [ ! -d $(INSTALLPREFIX)/etc/wanpipe/scripts ]; then \
		mkdir -p $(INSTALLPREFIX)/etc/wanpipe/scripts; \
	fi   
	@\cp -rf wan_ec  $(INSTALLPREFIX)/etc/wanpipe/
	@install -D -m 755 samples/wanrouter  $(INSTALLPREFIX)/usr/sbin/wanrouter
	@echo
	@echo "Wanpipe etc installed in $(INSTALLPREFIX)/etc/wanpipe";
	@echo

install_inc:
	@if [ -e $(INSTALLPREFIX)/usr/include/wanpipe ]; then \
		\rm -rf $(INSTALLPREFIX)/usr/include/wanpipe; \
        fi
	@\mkdir -p $(INSTALLPREFIX)/usr/include/wanpipe 
	@ln -s $(INSTALLPREFIX)/usr/include/wanpipe/ $(INSTALLPREFIX)/usr/include/wanpipe/linux
	@\cp -f $(PWD)/patches/kdrivers/include/*.h $(INSTALLPREFIX)/usr/include/wanpipe/
	@\cp -rf $(PWD)/patches/kdrivers/wanec/oct6100_api/include/ $(INSTALLPREFIX)/usr/include/wanpipe/oct6100_api	
	@\cp -rf $(PWD)/patches/kdrivers/wanec/*.h $(INSTALLPREFIX)/usr/include/wanpipe/

smgbri: 
	@cd ssmg/libsangoma.trunk; ./configure --prefix=/usr ; cd ../..;
	$(MAKE) -C ssmg/libsangoma.trunk/ CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	@cd ssmg/sangoma_mgd.trunk/lib/libteletone; ./configure --prefix=/usr ; cd ../../..;
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR) ASTDIR=$(ASTDIR)


clean_smgbri:
	$(MAKE) -C ssmg/libsangoma.trunk/ clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/lib/libteletone clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	$(MAKE) -C ssmg/sangoma_mgd.trunk/ clean CC=$(CC) PREFIX=$(INSTALLPREFIX) KDIR=$(KDIR)
	
