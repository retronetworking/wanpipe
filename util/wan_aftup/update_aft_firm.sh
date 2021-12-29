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

eval "./scripts/load.sh"

if [ "$cmd" == "" ]; then
	eval "./wan_aftup"
elif [ "$cmd" == "-auto" ]; then
	eval "./wan_aftup -auto"
else
	echo
	echo "Warning: Unknown command argument ($1)!"
	echo
fi

eval "./scripts/unload.sh"



