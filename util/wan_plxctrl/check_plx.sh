#!/bin/sh

cmd=$1;

eval "make clean > /dev/null"
eval "make > /dev/null"

if [ -e /proc/net/wanrouter ]; then

	echo
	echo "Warning: Wanpipe modules loaded"
	echo "         Please remove wanpipe modules from the kernel"
	echo
	echo "         eg: wanrouter stop"
	echo
	exit 1
fi


trap '' 2

eval "../wan_aftup/scripts/load.sh"

sleep 1

IFACES=`cat /proc/net/dev | cut -d":" -f 1 | grep "w.*g" | xargs`
cfg=0

for ifdev in $IFACES
do
#echo $ifdev
val=`./wan_plxup -i $ifdev  -r F`
echo "Reading $val from $ifdev"
usleep 50000
done

eval "../wan_aftup/scripts/unload.sh"


