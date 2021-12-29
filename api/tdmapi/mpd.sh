#!/bin/sh

cmd=$1

if  [ -z $cmd ]; then
  	echo "ERROR usage"
	exit 1
fi

devs="1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16"

for dev in $devs
do
 
 	wan_ec_client wanpipe$dev $cmd all

done
