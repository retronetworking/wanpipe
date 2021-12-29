all: 
	$(shell cd bwm-ng; ./configure > /dev/null)
	make -C bwm-ng

clean:
	make -C bwm-ng clean

install:
	@echo "Installing wpbwm in $(WAN_VIRTUAL)/usr/sbin"
	install -D bwm-ng/src/bwm-ng	  $(WAN_VIRTUAL)/usr/sbin/wpbwm

uninstall:
	@echo "Un-installing wpbwm from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/wpbwm

