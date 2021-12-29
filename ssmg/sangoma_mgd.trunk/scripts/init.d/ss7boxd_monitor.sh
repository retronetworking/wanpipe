#!/bin/bash

PROD="ss7boxd_monitor"
SIG="ss7boxd"




logit()
{
    local data="$1"

    echo "$PROD: $data"
    logger "$PROD: $data"
}



eval 'pidof $SIG' 2> /dev/null > /dev/null
if  [ $? -ne 0 ]; then
	logit "Error: $SIG not already started"
	exit 1
fi
	
logit "Service Started"

while [ 1 ];
do

	eval 'pidof $SIG' 2> /dev/null > /dev/null
	if  [ $? -ne 0 ]; then
		logit "$SIG not running..."	
		eval "nice /etc/init.d/smgss7_init_ctrl restart &"
		exit 1
	fi

	#logit "$SIG running..."	
	sleep 1

done
