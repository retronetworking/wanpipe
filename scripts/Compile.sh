
#!/bin/sh

make CFLAGS=" -Wp,-MD,.wanpipe.o.d -nostdinc -iwithprefix include -D__LINUX__ -Dlinux -D__KERNEL__ -I/lib/modules/2.6.18-92.1.22.el5/build/include  -DMODULE   -DAF_WANPIPE_2612_FORCE_UPDATE -DWANPIPE_MOD_266_FORCE_UPDATE   -m32 -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Wstrict-prototypes -Wundef -Werror-implicit-function-declaration -Os -pipe -msoft-float -fno-builtin-sprintf -fno-builtin-log2 -fno-builtin-puts  -mpreferred-stack-boundary=2  -march=i686 -mtune=generic -mtune=generic -mregparm=3 -ffreestanding -I/lib/modules/2.6.18-92.1.22.el5/build/include/asm-i386/mach-generic -I/lib/modules/2.6.18-92.1.22.el5/build/include/asm-i386/mach-default -fomit-frame-pointer -g  -fno-stack-protector -Wdeclaration-after-statement -Wno-pointer-sign -D__KERNEL__ -Iinclude  -include include/linux/autoconf.h    -I/usr/src/zaptel"  PROTOCOL_DEFINES="-DCONFIG_PRODUCT_WANPIPE_BASE  -DCONFIG_PRODUCT_WANPIPE_FR  -DCONFIG_PRODUCT_WANPIPE_CHDLC  -DCONFIG_PRODUCT_WANPIPE_PPP  -DCONFIG_PRODUCT_WANPIPE_X25  -DCONFIG_PRODUCT_WANPIPE_ADSL  -DCONFIG_PRODUCT_WANPIPE_LIP_ATM -DCONFIG_PRODUCT_WANPIPE_ATM  -DCONFIG_PRODUCT_WANPIPE_MULTFR  -DCONFIG_PRODUCT_WANPIPE_MULTPROT  -DCONFIG_PRODUCT_WANPIPE_AFT -DCONFIG_PRODUCT_WANPIPE_AFT_CORE -DCONFIG_PRODUCT_WANPIPE_AFT_TE1 -DCONFIG_PRODUCT_WANPIPE_AFT_56K -DCONFIG_PRODUCT_WANPIPE_AFT_RM -DCONFIG_PRODUCT_WANPIPE_CODEC_SLINEAR_LAW  -DCONFIG_PRODUCT_WANPIPE_AFT_BRI  -DCONFIG_PRODUCT_WANPIPE_AFT_SERIAL -DCONFIG_PRODUCT_WANPIPE_AFT_A600  -DCONFIG_PRODUCT_WANPIPE_AFT_TE3  -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE  -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN  -DCONFIG_PRODUCT_WANPIPE_TDM_VOICE_DCHAN_ZAPTEL  -DWANPIPE_USE_I_PRIVATE -DCONFIG_WANPIPE_HWEC "

