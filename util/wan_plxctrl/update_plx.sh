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
eval "echo $val | grep \"Reading 00 from\" > /dev/null 2> /dev/null"
if [ $? -ne 0 ]; then
	echo "Configuring $ifdev"
	./wan_plxup -i $ifdev -w F B
	cfg=1;
fi
usleep 50000
done

eval "../wan_aftup/scripts/unload.sh"

if [ $cfg -eq 1 ]; then

echo
echo "PLX Updated Successfuly"
echo "-----------------------"
echo "The machine must be SHUTDOWN/POWER OFF before"
echo "effects take place. A soft restart is not sufficent."
echo

else

echo
echo "PLX Update Failed"
echo "-----------------------"
echo "Please confirm you have a Sangoma PCI-Express card"
echo "installed. If the PCI-Express card IS installed please contact"
echo "Sangoma Tech Support, and send us the output of: lspci -v"
echo

fi



