#!/bin/sh

cmd=$1

if [ "$cmd" = "inuse" ]; then
	/usr/local/ss7box/ss7boost_cli --ckt-report --span all --chan all | grep -c "Y   Y"
elif [ "$cmd" = "free" ]; then
	/usr/local/ss7box/ss7boost_cli --ckt-report --span all --chan all | grep -c "Y   n"
else
	/usr/local/ss7box/ss7boost_cli --ckt-report --span all --chan all
fi

