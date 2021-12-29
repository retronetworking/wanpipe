#!/bin/sh


home=$(pwd)

BTDIR=oct6100_api/apilib/bt
LARGMATHDIR=oct6100_api/apilib/largmath
LLMANDIR=oct6100_api/apilib/llman
OCTAPIDIR=oct6100_api/octdeviceapi/oct6100api/oct6100_api
OCTAPIMIDIR=oct6100_api/octdeviceapi/oct6100api/oct6100_apimi

echo "HOME is $home"


EXTRA_FLAGS="-I. -I$home -I$home/oct6100_api -I$home/oct6100_api/include -I$home/oct6100_api/include  -I$home/oct6100_api/include/apilib -I$home/oct6100_api/include/apilib -I$home/oct6100_api/include/octrpc -I$home/oct6100_api/include/oct6100api -I$home/oct6100_api/octdeviceapi/oct6100api -DENABLE_TONE_PLAY "
					      

files="wanec_iface wanec_cmd wanec_utils wanec_dev $BTDIR/octapi_bt0 $LARGMATHDIR/octapi_largmath $LLMANDIR/octapi_llman $OCTAPIMIDIR/oct6100_mask_interrupts $OCTAPIDIR/oct6100_adpcm_chan $OCTAPIDIR/oct6100_channel $OCTAPIDIR/oct6100_chip_open $OCTAPIDIR/oct6100_chip_stats $OCTAPIDIR/oct6100_conf_bridge $OCTAPIDIR/oct6100_debug $OCTAPIDIR/oct6100_events $OCTAPIDIR/oct6100_interrupts $OCTAPIDIR/oct6100_memory $OCTAPIDIR/oct6100_miscellaneous  $OCTAPIDIR/oct6100_mixer $OCTAPIDIR/oct6100_phasing_tsst $OCTAPIDIR/oct6100_playout_buf $OCTAPIDIR/oct6100_remote_debug $OCTAPIDIR/oct6100_tlv $OCTAPIDIR/oct6100_tone_detection $OCTAPIDIR/oct6100_tsi_cnct $OCTAPIDIR/oct6100_tsst $OCTAPIDIR/oct6100_user " 

for file in $files
do
   	ofiles=$ofiles"$file.o "
done

echo "Compiling: $ofiles"

make MODULE_NAME=wanec OBJS="$ofiles" CC=gcc KBUILD_VERBOSE=0 KDIR=/lib/modules/$(uname -r)/build EXTRA_CFLAGS="-D__LINUX__ $EXTRA_FLAGS "
