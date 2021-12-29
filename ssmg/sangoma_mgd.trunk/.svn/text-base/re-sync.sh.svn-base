#!/bin/sh

home=`pwd`;

ss7boost="ss7boost"
ss7box="ss7boxd"

NICE=
RENICE=

if [ ! -e /usr/local/ss7box/$ss7boost ]; then
	echo "Error: ss7boost not found in /usr/local/ss7box dir";
	exit 1
fi
if [ ! -e /usr/local/ss7box/$ss7boxd ]; then
	echo "Error: ss7boxd not found in /usr/local/ss7box dir";
	exit 1
fi

eval "ulimit -n 65000"

sangoma_mgd -term

kill -TERM $(pidof asterisk);

if [ -f /var/run/sangoma_mgd.pid ]; then
	kill -KILL $(pidof sangoma_mgd)
	sangoma_mgd -term -wipe
fi

cd /usr/local/ss7box
./$ss7boost --term
sleep 1
#eval "$NICE ./$ss7boost --hb" 
eval "$NICE ./$ss7boost" 
sleep 5

for((i=0;i<60;i++))
do
	if [ -f /var/run/sangoma_mgd.pid ]; then
		break;
	fi
	echo "Waiting for smg to come up!"
	sleep 2
done

sleep 2

if [ "$RENICE" != "" ]; then
	echo "Running RNICE on ss7boost"
	eval "$RENICE $(pidof $ss7boost)"
	echo "Running RNICE on ss7box"
	eval "$RENICE $(pidof $ss7box)"
fi

cd $home


#exit 0;

if [ 1 -eq 0 ]; then
	ulimit -n 65536
	asterisk -c
else
	ulimit -n 65536
	eval "$NICE asterisk -g" 
	#Enable this to start asterisk in priority mode
	#in this case comment out the one above.
	#eval "$NICE asterisk -g -p"
	sleep 1
	asterisk -c -r
fi
