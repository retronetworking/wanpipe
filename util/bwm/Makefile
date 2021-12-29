CC=gcc
CFLAGS= -O2 -Wall

all: bwm.c
	$(CC) $(CFLAGS)  bwm.c -o wpbwm

clean:
	rm -f wpbwm

install:
	@echo "Installing wpbwm in $(WAN_VIRTUAL)/usr/sbin"
	install -D wpbwm	  $(WAN_VIRTUAL)/usr/sbin/wpbwm

uninstall:
	@echo "Un-installing wpbwm from $(WAN_VIRTUAL)/usr/sbin"
	rm -f $(WAN_VIRTUAL)/usr/sbin/wpbwm

