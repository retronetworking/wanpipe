#!/bin/sh

WAN_VIRTUAL=$2
WZDIR=${1:-usr/local/sbin/wancfg_zaptel}

if [ -z $WZDIR ]; then
	echo "Directory not found $WZDIR"  	
	exit 1
fi

if [ -e $WAN_VIRTUAL/$WZDIR ]; then 
	rm -rf $WAN_VIRTUAL/$WZDIR
fi 
