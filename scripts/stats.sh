#!/bin/bash

CMD=none
token=overrun
rec=0

while [ ! -z $1 ]
do
  	if [ "$1" = "clear" ]; then
      	CMD=clear
	fi
	if [ "$1" = "overrun" ]; then
      	token=overrun
	fi
	if [ "$1" = "ifstat" ]; then
      	token=packet
	fi
	if [ "$1" = "ifstat-rx" ]; then
      	token="RX.*packet"
	fi
	if [ "$1" = "ifstat-tx" ]; then
      	token="TX.*packet"
	fi
	if [ "$1" = "record" ]; then
      	rec=1
	fi
	shift
done

DEVS=$(cat /proc/net/dev | egrep "w.*g" | cut -d':' -f1 | xargs) 

#echo "$DEVS"

if [ $rec = 1 ]; then
echo $(date) >> stats.out
fi
for dev in $DEVS
do
	if [ "$CMD" = "clear" ]; then
		wanpipemon -i $dev -c fc
	fi	

	if [ "$token" = "overrun" ]; then
		line=`wanpipemon -i $dev -c sc | grep "$token"`
	else 
		line=`ifconfig $dev | grep "$token"`
	fi

	if [ $rec = 1 ] ; then
		echo "IF => $dev\n$line" | tee -a stats.out
	else
		echo -e "IF => $dev\n$line" 
	fi
done

