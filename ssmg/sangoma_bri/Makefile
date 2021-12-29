
INSTALLPREFIX=
ifndef ARCH
       ARCH=$(shell uname -m)
endif

SANGOMA_BRID=sangoma_brid.i686
ifeq ($(ARCH),x86_64)
	SANGOMA_BRID=sangoma_brid.x86_64
endif
all:

install:
	install -D $(SANGOMA_BRID) $(INSTALLPREFIX)/usr/sbin/sangoma_brid

	@echo
	@echo "Sangoma BRI Install Done"
	@echo "Run: /usr/sbin/wancfg_smg to configure wanpipe BRI"
	@echo


