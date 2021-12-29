/*
 ************************************************************************
 * wanpipe_includes.h							*
 *		WANPIPE(tm) 	Global includes for Sangoma drivers	*
 *									*
 * Author:	Alex Feldman <al.feldman@sangoma.com>			*
 *======================================================================*
 *	Aug 10 2002		Alex Feldman	Initial version		*
 *									*
 ************************************************************************
 */

#ifndef __WANPIPE_INCLUDES_H
# define __WANPIPE_INCLUDES_H

#if defined(__FreeBSD__)
/*
**		***	F R E E B S D	***
# include <stddef.h>
*/
# include <gnu/ext2fs/i386-bitops.h>
# include <sys/types.h>
# include <sys/param.h>
# include <sys/systm.h>
# include <sys/syslog.h>
# include <sys/conf.h>
# include <sys/errno.h>
# if (__FreeBSD_version > 400000)
#  include <sys/ioccom.h>
# else
#  include <i386/isa/isa_device.h> 
# endif
# if (__FreeBSD_version >= 410000)
#  include <sys/taskqueue.h>
# endif
# include <sys/malloc.h>
# include <sys/errno.h>
# include <sys/mbuf.h>
# include <sys/sockio.h>
# include <sys/ioctl_compat.h>
# include <sys/socket.h>
# include <sys/callout.h>
# include <sys/kernel.h>
# include <sys/time.h>
# include <sys/module.h>
# ifdef __SDLA_HW_LEVEL
#  include <machine/bus.h>
#  include <machine/resource.h>
#  include <sys/bus.h>
#  include <sys/rman.h>
#  include <sys/interrupt.h>
#  include <i386/isa/intr_machdep.h>
# endif
# include <sys/filio.h>
# include <sys/uio.h>
# include <sys/tty.h>
# include <sys/ttycom.h>
# include <sys/proc.h>
# include <pci/pcireg.h>
# include <pci/pcivar.h>
# include <net/bpf.h>
# include <net/bpfdesc.h>
# include <net/if_dl.h>
# include <net/if_types.h>
# include <net/if.h>
# include <net/if_media.h>
# include <net/if_ppp.h>
# include <net/if_sppp.h>
# include <net/netisr.h>
# include <net/route.h>
# if (__FreeBSD_version >= 501000)
#  include <net/netisr.h>
# elif (__FreeBSD_version > 400000)
#  include <net/intrq.h>
# endif
# include <netinet/in_systm.h>
# include <netinet/in.h>
# include <netinet/in_var.h>
# include <netinet/udp.h>
# include <netinet/ip.h>
# include <netinet/if_ether.h>
# include <netipx/ipx.h>
# include <netipx/ipx_if.h>
#ifdef NETGRAPH
# include <netgraph/ng_message.h>
# include <netgraph/netgraph.h>
#endif /* NETGRAPH */
# include <machine/param.h>
# if (__FreeBSD_version < 500000)
#  include <machine/types.h>
# endif
# include <machine/clock.h>
# include <machine/stdarg.h>
# include <machine/atomic.h>
# include <machine/clock.h>
# include <machine/bus.h>
# include <machine/md_var.h>
# include <vm/vm.h>
# include <vm/pmap.h>
# include <vm/vm_extern.h>
# include <vm/vm_kern.h>
#elif defined(__OpenBSD__)
/*
**		***	O P E N B S D	***
*/
# include </usr/include/bitstring.h>
# include <sys/types.h>
# include <sys/param.h>
# include <sys/systm.h>
# include <sys/syslog.h>
# include <sys/ioccom.h>
# include <sys/conf.h>
# include <sys/malloc.h>
# include <sys/errno.h>
# include <sys/exec.h>
# include <sys/lkm.h>
# include <sys/mbuf.h>
# include <sys/sockio.h>
# include <sys/socket.h>
# include <sys/kernel.h>
# include <sys/device.h>
# include <sys/time.h>
# include <sys/timeout.h>
# include <sys/tty.h>
# include <sys/ttycom.h>
# include <i386/bus.h>
# include <machine/types.h>
# include <machine/param.h>
# include <machine/cpufunc.h>
# include <machine/bus.h>
# include <machine/stdarg.h>
# include <net/bpf.h>
# include <net/bpfdesc.h>
# include <net/if_dl.h>
# include <net/if_types.h>
# include <net/if.h>
# include <net/netisr.h>
# include <net/route.h>
# include <net/if_media.h>
# include <net/ppp_defs.h>
# include <net/if_ppp.h>
# include <net/if_sppp.h>
# include <netinet/in_systm.h>
# include <netinet/in.h>
# include <netinet/in_var.h>
# include <netinet/if_ether.h>
# include <netinet/udp.h>
# include <netinet/ip.h>
# include <netipx/ipx.h>
# include <netipx/ipx_if.h>
# if defined(OpenBSD3_1) || defined(OpenBSD3_2) || defined(OpenBSD3_3) ||defined(OpenBSD3_4)
#  include <uvm/uvm_extern.h>
# elif defined(OpenBSD3_0)
#  include <vm/vm.h>
#  include <vm/pmap.h>
#  include <uvm/uvm_extern.h>
# else
#  include <vm/vm.h>
#  include <vm/pmap.h>
# endif
#elif defined(__LINUX__)
#ifdef __KERNEL__
/*
**		***	L I N U X	***
*/
# include <linux/init.h>
# include <linux/version.h>	/**/
# include <linux/config.h>	/* OS configuration options */
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
#  if !(defined __NO_VERSION__) && !defined(_K22X_MODULE_FIX_)
#   define __NO_VERSION__	
#  endif
# endif
# include <linux/module.h>
# include <linux/types.h>
# include <linux/sched.h>
# include <linux/mm.h>
# include <linux/slab.h>
# include <linux/stddef.h>	/* offsetof, etc. */
# include <linux/errno.h>	/* returns codes */
# include <linux/string.h>	/* inline memset, etc */
# include <linux/ctype.h>
# include <linux/kernel.h>	/* printk()m and other usefull stuff */
# include <linux/timer.h>
# include <linux/kmod.h>
# include <net/ip.h>
# include <net/protocol.h>
# include <net/sock.h>
# include <net/route.h>
# include <linux/fcntl.h>
# include <linux/skbuff.h>
# include <linux/socket.h>
# include <linux/poll.h>
# include <linux/wireless.h>
# include <linux/in.h>
# include <linux/inet.h>
# include <linux/netdevice.h>
# include <asm/io.h>		/* phys_to_virt() */
# include <asm/system.h>
# include <asm/byteorder.h>
# include <asm/delay.h>
# include <linux/pci.h>
# include <linux/if.h>
# include <linux/if_arp.h>
# include <linux/tcp.h>
# include <linux/ip.h>
# include <linux/udp.h>
# include <linux/ioport.h>
# include <linux/init.h>
# include <linux/pkt_sched.h>
# include <linux/etherdevice.h>
# include <linux/random.h>
# include <asm/uaccess.h>
# include <linux/inetdevice.h>
# include <linux/vmalloc.h>     /* vmalloc, vfree */
# include <asm/uaccess.h>        /* copy_to/from_user */
# include <linux/init.h>         /* __initfunc et al. */
# ifdef CONFIG_INET
#  include <net/inet_common.h>
# endif
#endif
#elif defined(__WINDOWS__)
/*
**		***	W I N D O W S	***
*/
# if defined(__KERNEL__)
#  include <ntddk.h>	/* PCI configuration struct */
#  include <ndis.h>		/* NDIS functions */
#  include <stdarg.h>
#  include <stdio.h>
#  include <stddef.h>	/* offsetof, etc. */
#  include <bit_win.h>	/* bit manipulation macros */
#  include <sang_status_defines.h>	// operation return codes
#  include <wanpipe_defines.h>		//for 'wan_udp_hdr_t'
#  include <sang_api.h>
# else
#  include <windows.h>
# endif
#else
# error "Unsupported Operating System!";
#endif

#endif	/* __WANPIPE_INCLUDES_H	*/

