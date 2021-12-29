SYSINC=/usr/src/linux/include

all:
	make -C ft1 all SYSINC=$(SYSINC) 
	make -C lxdialog all SYSINC=$(SYSINC)
	make -C sdladump all SYSINC=$(SYSINC)
	make -C wanconfig all SYSINC=$(SYSINC)
	make -C wanpipemon all SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS) 
	make -C wanpipemon_legacy all SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS) 
#	make -C wancfg all SYSINC=$(SYSINC) 
	make -C bwm all SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS)

install:
	make -C ft1 install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wancfg_legacy install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C lxdialog install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C sdladump install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanconfig install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon_legacy install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wancfg install WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C bwm install SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS)

uninstall:
	make -C ft1 uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wancfg_legacy uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C lxdialog uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C sdladump uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanconfig uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon_legacy uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wancfg uninstall WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C bwm uninstall SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS)

clean:
	make -C ft1 clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C lxdialog clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C sdladump clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanconfig clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wanpipemon_legacy clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wancfg clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C wan_aftup clean WAN_VIRTUAL=$(WAN_VIRTUAL)
	make -C bwm clean SYSINC=$(SYSINC) PROTOCOL_DEFS=$(PROTOCOL_DEFS)
