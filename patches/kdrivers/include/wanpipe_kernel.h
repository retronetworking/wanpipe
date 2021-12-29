
#ifndef _WANPIPE_KERNEL_H
#define _WANPIPE_KERNEL_H

#ifdef __KERNEL__

#include <linux/version.h>

#if 0
#pragma GCC diagnostic warning "-Wconversion"
#pragma GCC diagnostic warning "-Wextra"
#pragma GCC diagnostic warning "-Wcast-qual"
#pragma GCC diagnostic warning "-Wcast-align"
#endif

# if defined (__BIG_ENDIAN_BITFIELD) 
# define WAN_BIG_ENDIAN 1
# undef  WAN_LITTLE_ENDIAN
# else
# undef  WAN_BIG_ENDIAN 
# define WAN_LITTLE_ENDIAN 1
# endif 

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,9) 
# define MODULE_LICENSE(a)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,10)
# define snprintf(a,b,c,d...)	sprintf(a,c,##d)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,15)
# define wp_ip_rt_ioctl(_cmd_,_rptr_) -EINVAL
# define wp_devinet_ioctl(_cmd_,_rptr_) -EINVAL
#else
# define wp_ip_rt_ioctl(_cmd_,_rptr_) ip_rt_ioctl(_cmd_,_rptr_)   
# define wp_devinet_ioctl(_cmd_,_rptr_)  devinet_ioctl(_cmd_,_rptr_)
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28) 
#define device_create_drvdata(a,b,c,d,e) device_create(a,b,c,d,e) 
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
#define __wan_skb_reset_mac_header(skb)  skb_reset_mac_header(skb)
#define __wan_skb_reset_network_header(skb) skb_reset_network_header(skb)
#define __wan_skb_reset_tail_pointer(skb) skb_reset_tail_pointer(skb)
#define __wan_skb_tail_pointer(skb) skb_tail_pointer(skb)
#define __wan_skb_set_tail_pointer(skb,offset) skb_set_tail_pointer(skb,offset)
#define __wan_skb_end_pointer(skb) skb_end_pointer(skb)
#else
#define __wan_skb_reset_mac_header(skb) (skb->mac.raw = skb->data)
#define __wan_skb_reset_network_header(skb) (skb->nh.raw  = skb->data)
#define __wan_skb_reset_tail_pointer(skb) (skb->tail = skb->data)
#define __wan_skb_end_pointer(skb) ((skb)->end)
#define __wan_skb_tail_pointer(skb) ((netskb_t*)skb)->tail
#define __wan_skb_set_tail_pointer(skb,offset) ((skb)->tail = ((skb)->data + offset))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
#define cancel_work_sync(work) ({ cancel_work_sync(work); 0; })
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30)
# define WAN_DEV_NAME(device) dev_name(&(device->dev))
#else
#define WAN_DEV_NAME(device) device->dev.bus_id
#endif

/////////////2.6.35//////////////////////////////
#ifndef netdev_mc_count
#	define netdev_mc_count(dev) dev->mc_count
#endif

#ifdef CONFIG_RPS
#	define WAN_MC_LIST_ADDR(mclist) mclist->addr
#	define WAN_SK_SLEEP(sk) sk_sleep(sk)
#else
#       define WAN_MC_LIST_ADDR(mclist) mclist->dmi_addr
#       define WAN_SK_SLEEP(sk) sk->sk_sleep
#endif

#ifndef DMA_BIT_MASK
#define DMA_BIT_MASK(a)  DMA_32BIT_MASK
#endif

////////////////////////////////////////////////



//////////////////////
#ifdef HAVE_NET_DEVICE_OPS
#define WAN_DECLARE_NETDEV_OPS(_ops_name) static struct net_device_ops _ops_name = {0};

#define WAN_NETDEV_OPS_BIND(dev,_ops_name)  dev->netdev_ops = &_ops_name

#define WAN_NETDEV_OPS_INIT(dev,ops,wan_init)				ops.ndo_init = wan_init
#define WAN_NETDEV_OPS_OPEN(dev,ops,wan_open)				ops.ndo_open = wan_open
#define WAN_NETDEV_OPS_STOP(dev,ops,wan_stop)				ops.ndo_stop = wan_stop
#define WAN_NETDEV_OPS_XMIT(dev,ops,wan_send)				ops.ndo_start_xmit = wan_send
#define WAN_NETDEV_OPS_STATS(dev,ops,wan_stats)				ops.ndo_get_stats = wan_stats
#define WAN_NETDEV_OPS_TIMEOUT(dev,ops,wan_timeout)			ops.ndo_tx_timeout = wan_timeout
#define WAN_NETDEV_OPS_IOCTL(dev,ops,wan_ioctl)				ops.ndo_do_ioctl = wan_ioctl
#define WAN_NETDEV_OPS_MTU(dev,ops,wan_mtu)				ops.ndo_change_mtu = wan_mtu
#define WAN_NETDEV_OPS_CONFIG(dev,ops,wan_set_config)			ops.ndo_set_config = wan_set_config
#define WAN_NETDEV_OPS_SET_MULTICAST_LIST(dev,ops,wan_multicast_list)	ops.ndo_set_multicast_list = wan_multicast_list
//#define WAN_CHANGE_MTU(dev)						dev->netdev_ops->ndo_change_mtu
//#define WAN_XMIT(dev)                                               	dev->netdev_ops->ndo_start_xmit
//#define WAN_IOCTL(dev)						dev->netdev_ops->ndo_do_ioctl
#define WAN_NETDEV_TEST_XMIT(dev)					dev->netdev_ops->ndo_start_xmit
#define WAN_NETDEV_XMIT(skb,dev)					dev->netdev_ops->ndo_start_xmit(skb,dev)
#define WAN_NETDEV_TEST_IOCTL(dev)					dev->netdev_ops->ndo_do_ioctl
#define WAN_NETDEV_IOCTL(dev,ifr,cmd)					dev->netdev_ops->ndo_do_ioctl(dev,ifr,cmd)
#define WAN_NETDEV_TEST_MTU(dev)					dev->netdev_ops->ndo_change_mtu
#define WAN_NETDEV_CHANGE_MTU(dev,skb)					dev->netdev_ops->ndo_change_mtu(dev,skb)

#else
#define WAN_DECLARE_NETDEV_OPS(_ops_name) 
#define WAN_NETDEV_OPS_BIND(dev,_ops_name)
#define WAN_NETDEV_OPS_INIT(dev,ops,wan_init)				dev->init = wan_init
#define WAN_NETDEV_OPS_OPEN(dev,ops,wan_open)				dev->open = wan_open	
#define WAN_NETDEV_OPS_STOP(dev,ops,wan_stop)				dev->stop = wan_stop
#define WAN_NETDEV_OPS_XMIT(dev,ops,wan_send)				dev->hard_start_xmit = wan_send
#define WAN_NETDEV_OPS_STATS(dev,ops,wan_stats)				dev->get_stats = wan_stats
#define WAN_NETDEV_OPS_TIMEOUT(dev,ops,wan_timeout)			dev->tx_timeout = wan_timeout
#define WAN_NETDEV_OPS_IOCTL(dev,ops,wan_ioctl)				dev->do_ioctl = wan_ioctl
#define WAN_NETDEV_OPS_MTU(dev,ops,wan_mtu)				dev->change_mtu = wan_mtu
#define WAN_NETDEV_OPS_CONFIG(dev,ops,wan_set_config)			dev->set_config = wan_set_config
#define WAN_NETDEV_OPS_SET_MULTICAST_LIST(dev,ops,wan_multicast_list)	dev->set_multicast_list = wan_multicast_list
//#define WAN_CHANGE_MTU(dev)						dev->change_mtu
//#define WAN_XMIT(dev)                                                 dev->hard_start_xmit
//#define WAN_IOCTL(dev)                                                dev->do_ioctl
#define WAN_NETDEV_TEST_XMIT(dev)					dev->hard_start_xmit
#define WAN_NETDEV_XMIT(skb,dev)					dev->hard_start_xmit(skb,dev)
#define WAN_NETDEV_TEST_IOCTL(dev)					dev->do_ioctl
#define WAN_NETDEV_IOCTL(dev,ifr,cmd)					dev->do_ioctl(dev,ifr,cmd)
#define WAN_NETDEV_TEST_MTU(dev)					dev->change_mtu
#define WAN_NETDEV_CHANGE_MTU(dev,skb)					dev->change_mtu(dev,skb)

#endif
//////////////////////////

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) || defined(LINUX_FEAT_2624)
# ifndef LINUX_FEAT_2624
#  define LINUX_FEAT_2624 1
# endif
# define wan_dev_get_by_name(name) dev_get_by_name(&init_net,name)
# define wan_dev_get_by_index(idx) dev_get_by_index(&init_net,idx)
# define wan_init_net(name)  init_net.name

# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26) || defined(LINUX_FEAT_CONFIG_NET_NS) 
#  define	wan_sock_net(_x)	sock_net(_x)
# else
static __inline struct net *wan_sock_net(const struct sock *sk)
{
        return sk->sk_net;
}
# endif

#else
# define wan_dev_get_by_name(name) dev_get_by_name(name)
# define wan_dev_get_by_index(idx) dev_get_by_index(idx)
# define wan_init_net(name)  name
#endif


typedef int (wan_get_info_t)(char *, char **, off_t, int);

#ifndef IRQF_SHARED
#define IRQF_SHARED SA_SHIRQ
#endif

/*==========================================================================
   KERNEL 2.6.
 *==========================================================================*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
/* KERNEL 2.6.X */

 #define LINUX_2_6	
 #define netdevice_t struct net_device

 #define FREE_READ 1
 #define FREE_WRITE 0

 #define stop_net_queue(a) 	netif_stop_queue(a) 
 #define start_net_queue(a) 	netif_start_queue(a)
 #define is_queue_stopped(a)	netif_queue_stopped(a)
 #define wake_net_dev(a)	netif_wake_queue(a)
 #define is_dev_running(a)	netif_running(a)
 #define wan_dev_kfree_skb(a,b)	dev_kfree_skb_any(a)

 #define tq_struct		work_struct

 #define wan_call_usermodehelper(a,b,c)   call_usermodehelper(a,b,c,0)

 #define pci_present() 	1

 static inline int wan_schedule_task(struct tq_struct *tq)
 {
	return schedule_work(tq);
 }


 static inline int wan_task_cancel(struct tq_struct *tq)
 {
#if  LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
	return cancel_work_sync (tq);

#elif defined(work_clear_pending)
	return cancel_work_sync (tq);

#elif defined(WORK_STRUCT_NOAUTOREL)
	return 0;
#else
	int err;
	err=cancel_delayed_work(tq);
	flush_scheduled_work();
	return err;
#endif
 }

#if 1
#define MOD_INC_USE_COUNT try_module_get(THIS_MODULE)
#define MOD_DEC_USE_COUNT module_put(THIS_MODULE)
#else
#define MOD_INC_USE_COUNT 
#define MOD_DEC_USE_COUNT
#endif

#define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) {\
                             if (WAN_NET_RATELIMIT()) { \
                                 DEBUG_EVENT("%s:%d: wanpipe: ADMIN_CHECK: Failed !\n",__FUNCTION__,__LINE__);\
                             } \
                             return -EPERM; \
                             }\
                        }

 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)){\
                                  if (WAN_NET_RATELIMIT()) { \
                                  DEBUG_EVENT("%s:%d: wanpipe: NET_ADMIN_CHECK: Failed !\n",__FUNCTION__,__LINE__);\
                                  } \
                                  return -EPERM; \
                                 }\
                            }

 #define WAN_IRQ_CALL(fn,args,ret)	ret = fn args
 #define WAN_IRQ_RETURN(a) 		return a
 #define WAN_IRQ_RETVAL			irqreturn_t
 #define WAN_IRQ_RETVAL_DECL(ret)	irqreturn_t ret = WAN_IRQ_NONE
 #define WAN_IRQ_RETVAL_SET(ret, val)	ret = val
 #define WAN_IRQ_HANDLED 		IRQ_HANDLED
 #define WAN_IRQ_NONE	 		IRQ_NONE

 #define mark_bh(a)

 #define wan_clear_bit(a,b)  	    clear_bit((a),(unsigned long*)(b))
 #define wan_set_bit(a,b)    	    set_bit((a),(unsigned long*)(b))
 #define wan_test_bit(a,b)   	    test_bit((a),(unsigned long*)(b))
 #define wan_test_and_set_bit(a,b)  test_and_set_bit((a),(unsigned long*)(b))
 #define wan_test_and_clear_bit(a,b)  test_and_clear_bit((a),(unsigned long*)(b))

 #define dev_init_buffers(a)

 #define WP_PDE(_a)		PDE(_a)

     

 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,9)
 # define wp_rcu_read_lock(in_dev)     rcu_read_lock()
 # define wp_rcu_read_unlock(in_dev)   rcu_read_unlock()
 # define wp_readb(ptr)		       readb((void __iomem *)(ptr))
 # define wp_reads(ptr)		       reads((void __iomem *)(ptr))
 # define wp_readl(ptr)		       readl((void __iomem *)(ptr))
 # define wp_writeb(data,ptr)	       writeb(data,(void __iomem *)(ptr))
 # define wp_writew(data,ptr)	       writew(data,(void __iomem *)(ptr))
 # define wp_writel(data,ptr)	       writel(data,(void __iomem *)(ptr))
 # define wp_memset_io(ptr,data,len)   memset_io((void __iomem *)(ptr),data,len)
 #else
 # define wp_rcu_read_lock(in_dev)     read_lock_bh(&in_dev->lock) 
 # define wp_rcu_read_unlock(in_dev)   read_unlock_bh(&in_dev->lock) 
 # define wp_readb(ptr)		       readb((ptr))
 # define wp_reads(ptr)		       reads((ptr))
 # define wp_readl(ptr)		       readl((ptr))
 # define wp_writeb(data,ptr)	       writeb(data,(ptr))
 # define wp_writew(data,ptr)	       writew(data,(ptr))
 # define wp_writel(data,ptr)	       writel(data,(ptr))
 # define wp_memset_io(ptr,data,len)   memset_io((ptr),data,len)
 #endif

 #if LINUX_VERSION_CODE == KERNEL_VERSION(2,6,14) 
 #define htonl	__constant_htonl
 #define htons	__constant_htons
 #define ntohl  __constant_ntohl
 #define ntohs  __constant_ntohs
 #endif




/*==========================================================================
   KERNEL 2.4.X
 *==========================================================================*/

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
/* --------------------------------------------------
 * KERNEL 2.4.X 
 * -------------------------------------------------*/
 
 #define LINUX_2_4
 #define netdevice_t struct net_device

 #define FREE_READ 1
 #define FREE_WRITE 0

 #define stop_net_queue(a) 	netif_stop_queue(a) 
 #define start_net_queue(a) 	netif_start_queue(a)
 #define is_queue_stopped(a)	netif_queue_stopped(a)
 #define wake_net_dev(a)	netif_wake_queue(a)
 #define is_dev_running(a)	netif_running(a)
 #define wan_dev_kfree_skb(a,b)	dev_kfree_skb_any(a)
 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)

 #define __dev_get(a)		dev_get(a)

 static inline int wan_schedule_task(struct tq_struct *tq)
 {
	return schedule_task(tq);
 }

 static inline int wan_task_cancel(struct tq_struct *tq)
 {
	return 0;
 }

 #ifndef INIT_WORK
 # define INIT_WORK INIT_TQUEUE
 #endif

 #define wan_call_usermodehelper(a,b,c)   call_usermodehelper(a,b,c)

#define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) {\
                             if (WAN_NET_RATELIMIT()) { \
                                 DEBUG_EVENT("%s:%d: wanpipe: ADMIN_CHECK: Failed !\n",__FUNCTION__,__LINE__);\
                             } \
                             return -EPERM; \
                             }\
                        }

 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)){\
                                  if (WAN_NET_RATELIMIT()) { \
                                  DEBUG_EVENT("%s:%d: wanpipe: NET_ADMIN_CHECK: Failed !\n",__FUNCTION__,__LINE__);\
                                  } \
                                  return -EPERM; \
                                 }\
                            }

 #define WAN_IRQ_CALL(fn,args,ret)	fn args
 #define WAN_IRQ_RETURN(a)		return
 #define WAN_IRQ_RETVAL			void
 #define WAN_IRQ_RETVAL_DECL(ret)
 #define WAN_IRQ_RETVAL_SET(ret, val)
 #ifndef WAN_IRQ_NONE
 # define WAN_IRQ_NONE	(0)
 #endif

 #ifndef WAN_IRQ_HANDLED
 # define WAN_IRQ_HANDLED	(1)
 #endif

 #define wan_clear_bit(a,b)  	    clear_bit((a),(b))
 #define wan_set_bit(a,b)    	    set_bit((a),(b))
 #define wan_test_bit(a,b)   	    test_bit((a),(b))
 #define wan_test_and_set_bit(a,b)  test_and_set_bit((a),(b))
 #define wan_test_and_clear_bit(a,b)  test_and_clear_bit((a),(b))

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,21)
 	 return (struct proc_dir_entry *)inode->u.generic_ip;
 #else
 	 return (struct proc_dir_entry *)inode->i_private;
 #endif
 }

 #define wp_rcu_read_lock(in_dev)     read_lock_bh(&in_dev->lock) 
 #define wp_rcu_read_unlock(in_dev)   read_unlock_bh(&in_dev->lock) 
 #define wp_readb(ptr)		      readb((ptr))
 #define wp_reads(ptr)		      reads((ptr))
 #define wp_readl(ptr)		      readl((ptr))

 #define wp_writeb(data,ptr)	       writeb(data,(ptr))
 #define wp_writew(data,ptr)	       writew(data,(ptr))
 #define wp_writel(data,ptr)	       writel(data,(ptr))
 #define wp_memset_io(ptr,data,len)   memset_io((ptr),data,len)




/*==========================================================================
   KERNEL 2.2.X
 *==========================================================================*/


#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,1,0)
/*-----------------------------------------------------
 * KERNEL 2.2.X 
 * ---------------------------------------------------*/

 #define LINUX_2_1
 #define net_device  device
 #define netdevice_t struct device
 #define FREE_READ 1
 #define FREE_WRITE 0

 #define S_IRUGO	0

 #define __exit

 #ifndef get_order
 # define get_order(x) __get_order(x)
 #endif

 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)
 #define wan_dev_kfree_skb(a,b)	kfree_skb(a)
 #define dev_kfree_skb_any(a)   kfree_skb(a)

 #define netif_wake_queue(dev)   do { \
                                    clear_bit(0, &(dev)->tbusy); \
                                    mark_bh(NET_BH); \
                                } while(0)
 #define netif_start_queue(dev)  do { \
                                    (dev)->tbusy = 0; \
                                    (dev)->interrupt = 0; \
                                    (dev)->start = 1; \
                                } while (0)

 #define netif_stop_queue(dev)    (set_bit(0, &(dev)->tbusy))
 #define netif_running(dev)       (dev)->start
 #define netdevice_start(dev)     (dev)->start = 1
 #define netdevice_stop(dev)      (dev)->start = 0
 #define netif_queue_stopped(dev) (test_bit(0,&(dev)->tbusy))
 #define netif_set_tx_timeout(dev, tf, tm)

 #define stop_net_queue(dev) 	netif_stop_queue(dev) 
 #define start_net_queue(dev) 	netif_start_queue(dev)
 #define is_queue_stopped(dev)	netif_queue_stopped(dev)
 #define wake_net_dev(dev)	netif_wake_queue(dev)
 #define is_dev_running(dev)	netif_running(dev)

 #define dev_kfree_skb_irq(x)   kfree_skb(x)

 #define tasklet_struct 	tq_struct

 #define __dev_get(a)		dev_get(a)
 
 #ifndef DECLARE_WAITQUEUE
 #define DECLARE_WAITQUEUE(wait, current)	struct wait_queue wait = { current, NULL }
 #endif

 #define tasklet_kill(a)  { if ((a)->sync) {} }

 #define request_mem_region(addr, size, name)	((void *)1)
 #define release_mem_region(addr, size)
 #define pci_enable_device(x)           (0)
 #define pci_resource_start(dev, bar)   dev->base_address[bar]

 #define wp_rcu_read_lock(in_dev)    
 #define wp_rcu_read_unlock(in_dev)   
 #define wp_readb(ptr)		       readb((ptr))
 #define wp_reads(ptr)		       reads((ptr))
 #define wp_readl(ptr)		       readl((ptr))
 #define wp_writeb(data,ptr)	       writeb(data,(ptr))
 #define wp_writew(data,ptr)	       writew(data,(ptr))
 #define wp_writel(data,ptr)	       writel(data,(ptr))
 #define wp_memset_io(ptr,data,len)   memset_io((ptr),data,len)

 static inline void tasklet_hi_schedule(struct tasklet_struct *tasklet)
 {
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
 } 

 static inline void tasklet_schedule(struct tasklet_struct *tasklet)
 {
	queue_task(tasklet, &tq_immediate);
	mark_bh(IMMEDIATE_BH);
 }

 static inline void tasklet_init(struct tasklet_struct *tasklet,
				void (*func)(unsigned long),
				unsigned long data)
 {
	tasklet->next = NULL;
	tasklet->sync = 0;
	tasklet->routine = (void (*)(void *))func;
	tasklet->data = (void *)data;
 }

 static inline int wan_schedule_task(struct tq_struct *tq)
 {
	queue_task(tq, &tq_scheduler);
	return 0;
 }
 static inline int wan_task_cancel(struct tq_struct *tq)
 {
	return 0;
 }


 /* Setup Dma Memory size copied directly from 3c505.c */
 static inline int __get_order(unsigned long size)
 {
        int order;

        size = (size - 1) >> (PAGE_SHIFT - 1);
        order = -1;
        do {
                size >>= 1;
                order++;
        } while (size);
        return order;
 }

 typedef int (get_info_t)(char *, char **, off_t, int, int);

 #define ADMIN_CHECK()  {if (!capable(CAP_SYS_ADMIN)) return -EPERM;}
 #define NET_ADMIN_CHECK()  {if (!capable(CAP_NET_ADMIN)) return -EPERM;}

 #define WAN_IRQ_CALL(fn,args,ret)	fn args
 #define WAN_IRQ_RETURN(a)      	return
 #define WAN_IRQ_RETVAL			void
 #define WAN_IRQ_RETVAL_DECL(ret)
 #define WAN_IRQ_RETVAL_SET(ret, val)
 #ifndef WAN_IRQ_NONE
 # define WAN_IRQ_NONE      (0)
 #endif

 #ifndef WAN_IRQ_HANDLED
 # define WAN_IRQ_HANDLED   (1)
 #endif

 typedef unsigned long mm_segment_t;

 #ifndef INIT_WORK
 # define INIT_WORK INIT_TQUEUE
 #endif
 
 #define wan_clear_bit(a,b)  	    clear_bit((a),(b))
 #define wan_set_bit(a,b)    	    set_bit((a),(b))
 #define wan_test_bit(a,b)   	    test_bit((a),(b))
 #define wan_test_and_set_bit(a,b)  test_and_set_bit((a),(b))
 #define wan_test_and_clear_bit(a,b)  test_and_clear_bit((a),(b))

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
 #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
	 return (struct proc_dir_entry *)inode->u.generic_ip;
 #else
	 return (struct proc_dir_entry *)inode->i_private;
 #endif	
 }

#else
/* KERNEL 2.0.X */

 
 #define LINUX_2_0
 #define netdevice_t struct device

 static inline struct proc_dir_entry *WP_PDE(const struct inode *inode)
 {
 #if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18)
        return (struct proc_dir_entry *)inode->i_private;
 #else
        return (struct proc_dir_entry *)inode->u.generic_ip;
 #endif	
 }

 #define test_and_set_bit set_bit
 #define net_ratelimit() 1 

 #define stop_net_queue(a) 	(set_bit(0, &a->tbusy)) 
 #define start_net_queue(a) 	(clear_bit(0,&a->tbusy))
 #define is_queue_stopped(a)	(a->tbusy)
 #define wake_net_dev(a)	{clear_bit(0,&a->tbusy);mark_bh(NET_BH);}
 #define is_dev_running(a)	(test_bit(0,(void*)&a->start))
 #define wan_dev_kfree_skb(a,b) kfree_skb(a,b)  		 
 #define pci_get_device(a,b,c)  pci_find_device(a,b,c)
 #define spin_lock_init(a)
 #define spin_lock(a)
 #define spin_unlock(a)

 #define __dev_get(a)		dev_get(a)

 #define netif_wake_queue(dev)   do { \
                                    clear_bit(0, &dev->tbusy); \
                                    mark_bh(NET_BH); \
                                } while(0)
 #define netif_start_queue(dev)  do { \
                                    dev->tbusy = 0; \
                                    dev->interrupt = 0; \
                                    dev->start = 1; \
                                } while (0)
 #define netif_stop_queue(dev)   set_bit(0, &dev->tbusy)
 #define netif_running(dev)      dev->start
 #define netdevice_start(dev)    dev->start = 1
 #define netdevice_stop(dev)     dev->start = 0
 #define netif_set_tx_timeout(dev, tf, tm)
 #define dev_kfree_skb_irq(x)    kfree_skb(x)

 typedef int (write_proc_t)(char *, char **, off_t, int, int);

 #define net_device_stats	enet_statistics	

 static inline int copy_from_user(void *a, void *b, int len){
	int err = verify_area(VERIFY_READ, b, len);
	if (err)
		return err;
		
	memcpy_fromfs (a, b, len);
	return 0;
 }

 static inline int copy_to_user(void *a, void *b, int len){
	int err = verify_area(VERIFY_WRITE, b, len);
	if (err)
		return err;
	memcpy_tofs (a, b,len);
	return 0;
 }

 #define WAN_IRQ_CALL(fn,args,ret)	fn args
 #define WAN_IRQ_RETURN(a)      	return
 #define WAN_IRQ_RETVAL			void
 #define WAN_IRQ_RETVAL_DECL(ret)
 #define WAN_IRQ_RETVAL_SET(ret, val)
 
 #ifndef WAN_IRQ_NONE
 # define WAN_IRQ_NONE      (0)
 #endif
 
 #ifndef WAN_IRQ_HANDLED
 # define WAN_IRQ_HANDLED   (1)
 #endif

 typedef unsigned long mm_segment_t;

#endif

static inline int open_dev_check(netdevice_t *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,18)
	return is_dev_running(dev);
#else
	return 0;
#endif
}

#define WAN_IFQ_INIT(ifq, max_pkt)		skb_queue_head_init((ifq))
#define WAN_IFQ_DESTROY(ifq)
#define WAN_IFQ_PURGE(ifq)			wan_skb_queue_purge((ifq))
#define WAN_IFQ_DMA_PURGE(ifq)							\
	{ netskb_t *skb;							\
		while ((skb=wan_skb_dequeue((ifq))) != NULL) {			\
	        	if (skb_shinfo(skb)->frag_list || skb_shinfo(skb)->nr_frags) {	\
		        	DEBUG_EVENT("WARNING [%s:%d]: SKB Corruption!\n",	\
			                   __FUNCTION__,__LINE__);			\
	        		continue;						\
			}							\
			wan_skb_free(skb); \
		}								\
	}

#define WAN_IFQ_ENQUEUE(ifq, skb, arg, err)	skb_queue_tail((ifq), (skb))
#define WAN_IFQ_LEN(ifq)			skb_queue_len((ifq))

# define WAN_TASKLET_INIT(task, priority, func, arg)	\
	(task)->running = 0;				\
	tasklet_init(&(task)->task_id,func,(unsigned long)arg) 

# define WAN_TASKLET_SCHEDULE(task)					\
	wan_set_bit(0, &(task)->running);				\
	tasklet_schedule(&(task)->task_id);			
#if 0
# define WAN_WP_TASKLET_SCHEDULE_PER_CPU(task,cpu)			\
	wan_set_bit(0, &(task)->running);				\
	wp_tasklet_hi_schedule_per_cpu(&(task)->task_id,cpu);		
#endif

# define WAN_TASKLET_RUNNING(task)					\
		wan_test_bit(0, &(task)->running)

# define WAN_TASKLET_END(task)	wan_clear_bit(0, &(task)->running)

# define WAN_TASKLET_KILL(task)	tasklet_kill(&(task)->task_id)

/* Due to 2.6.20 kernel the wan_taskq_t is now a direct
 * workqueue struct not an abstracted structure */
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)) 
#  define WAN_TASKQ_INIT(task, priority, func, arg)	\
		INIT_WORK((task),func,arg) 
# else
#  define WAN_TASKQ_INIT(task, priority, func, arg)	\
		INIT_WORK((task),func)	
# endif
# define WAN_IS_TASKQ_SCHEDULE

#if 0
# define WAN_TASKQ_SCHEDULE(task)			\
	{ int err; \
		err =wan_schedule_task(task); \
	if (err) { \
		printk(KERN_INFO "%s:%d Error: Failed to schedule a Task (err=%i)!\n", 	__FUNCTION__,__LINE__,err);   \
	} \
	}
#else
# define WAN_TASKQ_SCHEDULE(task)			\
		wan_schedule_task(task); 
#endif


# define WAN_TASKQ_STOP(task) \
	wan_task_cancel(task)

#else /* __KERNEL__ */

#include <linux/version.h>

/* This file is not being included from kernel space
 * we need to define what kersdladrv_pci_tblnel version we are
 * running */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	#define LINUX_2_6
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,0)
 	#define LINUX_2_4
#else
	#define LINUX_2_4
#endif



#endif


/* For All operating Systems */

#pragma pack(1)         

typedef struct wan_iovec
{
	uint32_t iov_len; /* Must be size_t (1003.1g) */
	void *iov_base;	/* BSD uses caddr_t (1003.1g requires void *) */
#ifndef __x86_64__
    uint32_t reserved;
#endif
}wan_iovec_t;

typedef struct wan_msghdr {
	uint32_t	msg_iovlen;	/* Number of blocks		*/
	wan_iovec_t *	msg_iov;	/* Data blocks			*/
#ifndef __x86_64__
    uint32_t reserved;
#endif
}wan_msghdr_t;          

static inline int wp_linux_strncasecmp(const char *s1, const char *s2, size_t n)
{
  if (n == 0)
    return 0;

  while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
      if (n == 0 || *s1 == '\0' || *s2 == '\0')
	break;
      s1++;
      s2++;
    }

  return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}

#pragma pack()

#if defined(__KERNEL__)

static __inline int wan_verify_iovec(wan_msghdr_t *m, wan_iovec_t *iov, char *address, int mode)
{
	int size, err, ct;
	
	if (m->msg_iovlen == 0) {
		return -EMSGSIZE;
	}
	
	size = m->msg_iovlen * sizeof(wan_iovec_t);

	
	if (copy_from_user(iov, m->msg_iov, size))
		return -EFAULT;

	m->msg_iov = iov;
	err = 0;

	for (ct = 0; ct < m->msg_iovlen; ct++) {
		err += iov[ct].iov_len;
		/*
		 * Goal is not to verify user data, but to prevent returning
		 * negative value, which is interpreted as errno.
		 * Overflow is still possible, but it is harmless.
		 */
		if (err < 0)
			return -EMSGSIZE;
	}

	return err;
}
      
 /*
 *	Copy iovec to kernel. Returns -EFAULT on error.
 *
 *	Note: this modifies the original iovec.
 */
 
static __inline int wan_memcpy_fromiovec(unsigned char *kdata, wan_iovec_t *iov, int len)
{
	while (len > 0) {
		if (iov->iov_len) {
			int copy = min_t(unsigned int, len, iov->iov_len);
			if (copy_from_user(kdata, iov->iov_base, copy))
				return -EFAULT;
			len -= copy;
			kdata += copy;
			iov->iov_base += copy;
			iov->iov_len -= copy;
		}
		iov++;
	}

	return 0;
}                

/*
 *	Copy kernel to iovec. Returns -EFAULT on error.
 *
 *	Note: this modifies the original iovec.
 */
 
static __inline int wan_memcpy_toiovec(wan_iovec_t *iov, unsigned char *kdata, int len)
{
	while (len > 0) {
		if (iov->iov_len) {
			int copy = min_t(unsigned int, iov->iov_len, len);
			if (copy_to_user(iov->iov_base, kdata, copy))
				return -EFAULT;
			kdata += copy;
			len -= copy;
			iov->iov_len -= copy;
			iov->iov_base += copy;
		}
		iov++;
	}

	return 0;
}             


#endif



#endif


