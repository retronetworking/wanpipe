#ifndef __WAN_AFTEN__
#define __WAN_AFTEN__

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
# include <net/wanpipe_includes.h>
# include <net/wanpipe_cfg.h>
# include <net/wanpipe_debug.h>
# include <net/wanpipe_defines.h>
# include <net/wanpipe_common.h>
# include <net/wanpipe_abstr.h>
# include <net/wanpipe_snmp.h>
# include <net/sdlapci.h>
# include <net/if_wanpipe_common.h>
# include <net/wanpipe_iface.h>
#else
# include <linux/wanpipe_includes.h>
# include <linux/wanpipe_debug.h>
# include <linux/wanpipe_defines.h>
# include <linux/wanpipe_common.h>
# include <linux/if_wanpipe.h>
# include <linux/sdlapci.h>
# include <linux/sdladrv.h>
# include <linux/wanpipe.h>
# include <linux/if_wanpipe_common.h>
//# include <linux/wanpipe_lip_kernel.h>
# include <linux/wanpipe_iface.h>
#endif

#define WAN_AFTEN_VER	1

struct wan_aften_priv {
	wanpipe_common_t	common;
	unsigned int		base_class;
	unsigned int		base_addr0;
	unsigned int		base_addr1;
	unsigned int		irq;
	unsigned int		slot;
	unsigned int		bus;
};

#endif
