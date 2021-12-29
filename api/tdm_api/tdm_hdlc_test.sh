#!/bin/sh

echo "ARGS $1 $2"

ARG1=$1;
ARG2=$2;


if [ "$ARG1" != "" ]; then
WANPIPES="$ARG1"
else
WANPIPES="1"
fi

if [ "$ARG2" != "" ]; then
IFACE_START=$ARG2 
IFACE_STOP=$ARG2
else
IFACE_START=1
IFACE_STOP=31
fi


CMD="wanpipe1"

for wanpipe_num in $WANPIPES
do	
	for ((i=$IFACE_START;i<=$IFACE_STOP;i+=1)); do
#CMD=$CMD" w"$wanpipe_num"g"$i   
CMD=$CMD" s"$wanpipe_num"c"$i   
	done
done

echo "$CMD " 

./aft_tdm_hdlc_test  $CMD 

