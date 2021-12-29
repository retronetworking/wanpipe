#!/bin/sh

eval "find /etc -name 'S*wanrouter' | xargs rm" 2> /dev/null > /dev/null
eval "find /etc -name 'K*wanrouter' | xargs rm" 2> /dev/null > /dev/null

if [ -e /usr/sbin/smgss7_ctrl ]; then 
	rm -f /usr/sbin/smgss7_ctrl 
fi
if [ -e /usr/sbin/smgss7_init_ctrl ]; then 
	rm -f /usr/sbin/smgss7_init_ctrl
fi
