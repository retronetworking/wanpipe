/*
 * Copyright (c) 2002
 *	Alex Feldman <al.feldman@sangoma.com>.  All rights reserved.
 *
 *	$Id: wanpipe_common.h,v 1.1 2006/07/28 20:58:18 sangoma Exp $
 */

/****************************************************************************
 * wanpipe_common.h	WANPIPE(tm) Multiprotocol WAN Link Driver.
 *
 * Author: 	Alex Feldman <al.feldman@sangoma.com>
 *
 * ==========================================================================
 * July 17, 2002 Alex Feldman 	Initial Version
 ****************************************************************************
 */

#ifndef	__WANPIPE_COMMON_H
# define	__WANPIPE_COMMON_H

#ifdef __LINUX__
# include <linux/wanpipe_kernel.h>
#endif

#ifdef __WINDOWS__
//#define DBG_WANPIPE_COMMON	DbgPrint
#define DBG_WANPIPE_COMMON

#include <wanpipe_cfg.h>

#if defined(__KERNEL__)
 #include <irql_check.h>
#endif
#endif

#ifdef WAN_DEBUG_MEM
extern atomic_t wan_debug_mem;
#endif

/*
*****************************************************************************
**			D E F I N E S				
*****************************************************************************
*/
#ifndef NIPQUAD
# define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#endif



#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define WAN_LIST_HEAD(name, type)			LIST_HEAD(name, type)
# define WAN_LIST_HEAD_INITIALIZER(head)		LIST_HEAD_INITIALIZER(head)
# define WAN_LIST_ENTRY(type) 				LIST_ENTRY(type)
# define WAN_LIST_EMPTY(head)				LIST_EMPTY(head)
# define WAN_LIST_FIRST(head)				LIST_FIRST(head)		
# define WAN_LIST_FOREACH(var, head, field)		LIST_FOREACH(var, head, field)
# define WAN_LIST_INIT(head)				LIST_INIT(head)
# define WAN_LIST_INSERT_AFTER(listelm, elm, field)	LIST_INSERT_AFTER(listelm, elm, field)
/*# define WAN_LIST_INSERT_BEFORE(listelm, elm, field)	LIST_INSERT_BEFORE(listelm, elm, field)*/
# define WAN_LIST_INSERT_HEAD(head, elm, field) 	LIST_INSERT_HEAD(head, elm, field)
# define WAN_LIST_NEXT(elm, field)			LIST_NEXT(elm, field)
# define WAN_LIST_REMOVE(elm, field)			LIST_REMOVE(elm, field)

#elif defined(__LINUX__)
	
# define WAN_LIST_HEAD(name, type)		struct name { struct type * lh_first; }
# define WAN_LIST_HEAD_INITIALIZER(head)	{ NULL }
# define WAN_LIST_ENTRY(type) 			struct { struct type *le_next; struct type **le_prev; }
# define WAN_LIST_FIRST(head)			((head)->lh_first)
# define WAN_LIST_END(head)			NULL
# define WAN_LIST_EMPTY(head)			(WAN_LIST_FIRST(head) == WAN_LIST_END(head))
# define WAN_LIST_NEXT(elm, field)		((elm)->field.le_next)
# define WAN_LIST_FOREACH(var, head, field)	for((var) = WAN_LIST_FIRST(head);	\
							(var);				\
							(var) = WAN_LIST_NEXT(var, field))
# define WAN_LIST_INIT(head)		do { WAN_LIST_FIRST(head) = NULL;}\
		while(0)

#define	WAN_LIST_INSERT_HEAD(head, elm, field) do {				\
	if ((WAN_LIST_NEXT((elm), field) = WAN_LIST_FIRST((head))) != NULL)	\
		WAN_LIST_FIRST((head))->field.le_prev = &WAN_LIST_NEXT((elm), field);\
	WAN_LIST_FIRST((head)) = (elm);					\
	(elm)->field.le_prev = &WAN_LIST_FIRST((head));			\
} while (0)
#define	WAN_LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if ((WAN_LIST_NEXT((elm), field) = WAN_LIST_NEXT((listelm), field)) != NULL)\
		WAN_LIST_NEXT((listelm), field)->field.le_prev =		\
		    &WAN_LIST_NEXT((elm), field);				\
	WAN_LIST_NEXT((listelm), field) = (elm);				\
	(elm)->field.le_prev = &WAN_LIST_NEXT((listelm), field);		\
} while (0)
#define	WAN_LIST_REMOVE(elm, field) do {					\
	if (WAN_LIST_NEXT((elm), field) != NULL)				\
		WAN_LIST_NEXT((elm), field)->field.le_prev = 		\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = WAN_LIST_NEXT((elm), field);		\
} while (0)

#elif defined(__WINDOWS__)
	
# define WAN_LIST_HEAD(name, type)			struct name { struct type * lh_first; }
# define WAN_LIST_HEAD_INITIALIZER(head)	{ NULL }
# define WAN_LIST_ENTRY(type) 				struct { struct type *le_next; struct type **le_prev; }

# define WAN_LIST_FIRST(head)			((head)->lh_first)
# define WAN_LIST_END(head)				NULL

# define WAN_LIST_EMPTY(head)			(WAN_LIST_FIRST(head) == WAN_LIST_END(head))
# define WAN_LIST_NEXT(elm, field)		((elm)->field.le_next)

# define WAN_LIST_FOREACH(var, head, field)	for((var) = WAN_LIST_FIRST(head);	\
												(var);							\
												(var) = WAN_LIST_NEXT(var, field))

#define WAN_LIST_INIT(head)		do {	\
	WAN_LIST_FIRST(head) = NULL;		\
}while(0)

#define	WAN_LIST_INSERT_HEAD(head, elm, field) do {								\
	if ((WAN_LIST_NEXT((elm), field) = WAN_LIST_FIRST((head))) != NULL)			\
		WAN_LIST_FIRST((head))->field.le_prev = &WAN_LIST_NEXT((elm), field);	\
	WAN_LIST_FIRST((head)) = (elm);												\
	(elm)->field.le_prev = &WAN_LIST_FIRST((head));								\
} while (0)

#define	WAN_LIST_INSERT_AFTER(listelm, elm, field) do {							\
	if ((WAN_LIST_NEXT((elm), field) = WAN_LIST_NEXT((listelm), field)) != NULL)\
		WAN_LIST_NEXT((listelm), field)->field.le_prev =		\
			&WAN_LIST_NEXT((elm), field);						\
	WAN_LIST_NEXT((listelm), field) = (elm);					\
	(elm)->field.le_prev = &WAN_LIST_NEXT((listelm), field);	\
} while (0)

#define	WAN_LIST_REMOVE(elm, field) do {									\
	if (WAN_LIST_NEXT((elm), field) != NULL)								\
		WAN_LIST_NEXT((elm), field)->field.le_prev = (elm)->field.le_prev;	\
	*(elm)->field.le_prev = WAN_LIST_NEXT((elm), field);					\
} while (0)

#else
# error "WAN_LISTx macros not supported yet!"
#endif




#if defined(__FreeBSD__)
# define WAN_TAILQ_FIRST(ifp)	TAILQ_FIRST(&ifp->if_addrhead)
# define WAN_TAILQ_NEXT(ifa)	TAILQ_NEXT(ifa, ifa_link)
#elif defined (__OpenBSD__)
# define WAN_TAILQ_FIRST(ifp)	TAILQ_FIRST(&ifp->if_addrlist)
# define WAN_TAILQ_NEXT(ifa)	TAILQ_NEXT(ifa, ifa_list)
#elif defined(__LINUX__)
#elif defined(__WINDOWS__)
#else
# error "WAN_TAILQ_x macros doesn't supported yet!"
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# ifndef IFQ_SET_READY
#  define IFQ_SET_READY(ifq)		/* nothing */
# endif
# ifndef IFQ_IS_EMPTY
#  define IFQ_IS_EMPTY(ifq)		((ifq)->ifq_len == 0)
# endif
# ifndef IFQ_INC_LEN
#  define IFQ_INC_LEN(ifq)		((ifq)->ifq_len++)
# endif
# ifndef IFQ_DEC_LEN
#  define IFQ_DEC_LEN(ifq)		(--(ifq)->ifq_len)
# endif
# ifndef IFQ_INC_DROPS
#  define IFQ_INC_DROPS(ifq)		((ifq)->ifq_drops++)
# endif
# ifndef IFQ_SET_MAXLEN
#  define IFQ_SET_MAXLEN(ifq, len)	((ifq)->ifq_maxlen = (len))
# endif
# if (__FreeBSD_version > 500000)
#  define IF_DROP	_IF_DROP
#  define IF_QFULL	_IF_QFULL
# endif
# ifndef IFQ_PURGE
#  if (__FreeBSD_version > 500000)
#   define IFQ_PURGE(ifq)	IF_DRAIN((ifq))
#  else
#   define IFQ_PURGE(ifq)					\
	while (1) {						\
		struct mbuf *m0;				\
		IF_DEQUEUE((ifq), m0);				\
		if (m0 == NULL)					\
			break;					\
		else						\
			m_freem(m0);				\
	}
#  endif
# endif
# ifndef IFQ_ENQUEUE
#  if (__FreeBSD_version > 500000)
#   define IFQ_ENQUEUE(ifq, m, pattr, err)	IF_ENQUEUE((ifq), (m))
#  else
#   define IFQ_ENQUEUE(ifq, m, pattr, err)			\
	do {							\
		if (IF_QFULL((ifq))) {				\
			m_freem((m));				\
			(err) = ENOBUFS;			\
		} else {					\
			IF_ENQUEUE((ifq), (m));			\
			(err) = 0;				\
		}						\
		if ((err))					\
			(ifq)->ifq_drops++;			\
	} while (0)
#  endif
# endif
# ifndef IFQ_DEQUEUE
#  define IFQ_DEQUEUE(ifq, m)	IF_DEQUEUE((ifq), (m))
# endif
# ifndef IFQ_POLL
#  define IFQ_POLL(ifq, m)	((m) = (ifq)->ifq_head)
# endif
# ifndef IFQ_CLASSIFY
struct altq_pktattr { void* attr; };
#  define IFQ_CLASSIFY(ifq, m, sa_family, pktattr) (pktattr)->attr = NULL
# endif
# if (__FreeBSD_version > 500000)
#  define IFQ_INIT(ifq, max_pkt)		\
		IFQ_SET_MAXLEN((ifq), max_pkt);	\
		mtx_init(&(ifq)->ifq_mtx, "ifq", "wan", MTX_DEF);	\
 		(ifq)->ifq_len = 0;
# else
#  define IFQ_INIT(ifq, max_pkt)		\
		IFQ_SET_MAXLEN((ifq), max_pkt);	\
 		(ifq)->ifq_len = 0;
# endif
# define IFQ_LEN(ifq)	((ifq)->ifq_len)
#elif defined(__LINUX__)
# define IFQ_INIT(trace_queue, max_pkt)			skb_queue_head_init(trace_queue)
# define IFQ_PURGE(trace_queue)				skb_queue_purge(trace_queue)
# define IFQ_ENQUEUE(trace_queue, skb, arg, err)	skb_queue_tail(trace_queue, skb)
# define IFQ_LEN(trace_queue)				skb_queue_len(trace_queue)
#elif defined(__WINDOWS__)
#else
# error "Undefined IFQ_x macros!"
#endif

#if defined(__FreeBSD__)
# if (__FreeBSD_version < 410000)
#  define WAN_TASKLET_INIT(task, priority, func, arg)	\
	task->running = 0;				\
	task->task_func = func; task->data = arg
#  define WAN_TASKLET_SCHEDULE(task)					\
	if (!test_bit(0, &task->running)){				\
		set_bit(0, &task->running);				\
		task->task_func(task->data, 0);				\
	}
#  define WAN_TASKLET_END(task)	clear_bit(0, &task->running)
#  define WAN_TASKLET_KILL(task)
# else
#  define WAN_TASKLET_INIT(task, priority, func, arg)	\
	task->running = 0;				\
	TASK_INIT(&task->task_id, priority, func, arg)
#  define WAN_TASKLET_SCHEDULE(task)					\
	if (!test_bit(0, &task->running)){				\
		set_bit(0, &task->running);				\
		taskqueue_enqueue(taskqueue_swi, &task->task_id);	\
	}
/*		taskqueue_run(taskqueue_swi);				\*/
#  define WAN_TASKLET_END(task)	clear_bit(0, &task->running)
#  define WAN_TASKLET_KILL(task)
# endif
#elif defined(__OpenBSD__)
# define WAN_TASKLET_INIT(task, priority, func, arg)	\
	task->running = 0;				\
	task->task_func = func; task->data = arg
# define WAN_TASKLET_SCHEDULE(task)					\
	if (!test_bit(0, &task->running)){				\
		set_bit(0, &task->running);				\
		task->task_func(task->data, 0);				\
	}
# define WAN_TASKLET_END(task)	clear_bit(0, &task->running)
# define WAN_TASKLET_KILL(task)
#elif defined(__LINUX__)
# define WAN_TASKLET_INIT(task, priority, func, arg)	\
	task->running = 0;				\
	tasklet_init(&task->task_id,func,(unsigned long)arg) 
# define WAN_TASKLET_SCHEDULE(task)					\
	tasklet_hi_schedule(&task->task_id);			
# define WAN_TASKLET_END(task)	clear_bit(0, &task->running)
# define WAN_TASKLET_KILL(task)	tasklet_kill(&task->task_id)
#elif defined(__WINDOWS__)

# define WAN_TASKLET_INIT(task, priority, func, arg)	\
{														\
	int rc;												\
	VERIFY_PASSIVE_IRQL(rc);							\
	if(rc == 0){										\
		KeInitializeDpc(&task->tqueue, func, (void*)arg);\
	}													\
}

# define WAN_TASKLET_SCHEDULE(task)					\
{													\
	int rc;											\
	VERIFY_HIGHER_THAN_DISPATCH_IRQL(rc);			\
	if(rc == 0){									\
		KeInsertQueueDpc(&task->tqueue, NULL, NULL);\
	}												\
}

# define WAN_TASKLET_END(task)	DBG_NOT_IMPL(("WAN_TASKLET_END()\n"));

# define WAN_TASKLET_KILL(task)	DBG_NOT_IMPL(("WAN_TASKLET_KILL()\n"));

#else
# error "Undefined WAN_TASKLET_x macro!"
#endif

#if defined(__FreeBSD__)
# if (__FreeBSD_version < 410000)
#  define WAN_TASKQ_INIT(task, priority, func, arg)	\
		task->tfunc = func; task->data = arg
# else
#  define WAN_TASKQ_INIT(task, priority, func, arg)	\
		TASK_INIT(&task->tqueue, priority, func, arg)
# endif
#elif defined(__OpenBSD__)
# define WAN_TASKQ_INIT(task, priority, func, arg)	\
		task->tfunc = func; task->data = arg
#elif defined(__LINUX__)
# define WAN_TASKQ_INIT(task, priority, func, arg)	\
		task->tqueue.sync = 0; 			\
		task->tqueue.routine = func;		\
		task->tqueue.data = arg
#elif defined(__WINDOWS__)

# define WAN_TASKQ_INIT(task, priority, func, arg)	\
	WAN_TASKLET_INIT(task, priority, func, arg)

#else
# error "Undefined WAN_TASKQ_INIT macro!"
#endif

#if defined(__FreeBSD__) && (__FreeBSD_version >= 410000)
# define WAN_TASKQ_SCHEDULE(task)			\
	taskqueue_enqueue(taskqueue_swi, &task->tqueue);\
	taskqueue_run(taskqueue_swi)
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
# define WAN_TASKQ_SCHEDULE(task)			\
	task->tfunc(task->data, 0)
#elif defined(__LINUX__)
# define WAN_TASKQ_SCHEDULE(task)			\
	schedule_task(&task->tqueue)
#elif defined(__WINDOWS__)

# define WAN_TASKQ_SCHEDULE(task)			\
	DBG_NOT_IMPL(("WAN_TASKQ_SCHEDULE()\n"))

	//queue the DPC
	//KeInsertQueueDpc(task);

#else
# error "Undefined WAN_TASKQ_SCHEDULE macro!"
#endif


#if defined(__LINUX__)
# define WAN_COPY_FROM_USER(k,u,l)	copy_from_user(k,u,l)
# define WAN_COPY_TO_USER(u,k,l)	copy_to_user(u,k,l)
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
# define WAN_COPY_FROM_USER(k,u,l)	copyin(u,k,l)
# define WAN_COPY_TO_USER(u,k,l)	copyout(k,u,l)
#elif defined(__WINDOWS__)
//
//defined in 'wanpipe_abstr.h'
//
#else
# error "Undefined WAN_COPY_FROM_USER/WAN_COPY_TO_USER macros!"
#endif

#if defined(__LINUX__)
# if (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43))
#  define WAN_NETIF_WAKE_QUEUE(dev)	do {				\
					clear_bit(0, &dev->tbusy);	\
					mark_bh(NET_BH);		\
					} while(0)
#  define WAN_NETIF_START_QUEUE(dev)	do {				\
					dev->tbusy = 0;			\
					dev->interrupt = 0;		\
					dev->start = 1;			\
					} while(0);
#  define WAN_NETIF_STOP_QUEUE(dev)	set_bit(0, &dev->tbusy)
#  define WAN_NETIF_RUNNING(dev)	dev->start
#  define WAN_NETDEVICE_START(dev)	dev->start = 1
#  define WAN_NETDEVICE_STOP(dev)	dev->start = 0
#  define WAN_NETIF_QUEUE_STOPPED(dev)	test_bit(0,&dev->tbusy)
# else
#  define WAN_NETIF_WAKE_QUEUE(dev)	netif_wake_queue(dev)
#  define WAN_NETIF_START_QUEUE(dev)	netif_start_queue(dev)
#  define WAN_NETIF_STOP_QUEUE(dev)	netif_stop_queue(dev)
#  define WAN_NETIF_RUNNING(dev)	netif_running(dev)
#  define WAN_NETDEVICE_START(dev)
#  define WAN_NETDEVICE_STOP(dev)
#  define WAN_NETIF_QUEUE_STOPPED(dev)	netif_queue_stopped(dev)
# endif
#define WAN_NETIF_UP(dev)		((dev)->flags&IFF_UP)

#elif defined(__FreeBSD__) || defined(__OpenBSD__)
# define WAN_NETIF_QUEUE_STOPPED(dev)	0
# define WAN_NETIF_WAKE_QUEUE(dev)
# define WAN_NETIF_STOP_QUEUE(dev)	
# define WAN_NETDEVICE_STOP(dev)
# define WAN_NETDEVICE_START(dev)
# define WAN_NETIF_RUNNING(dev)	
# define WAN_NETIF_START_QUEUE(dev)
# define MOD_INC_USE_COUNT
# define MOD_DEC_USE_COUNT
# define WAN_NETIF_UP(dev)		((dev)->if_flags&IFF_UP)
#elif defined(__WINDOWS__)

/*
#define WAN_NETIF_WAKE_QUEUE(a)		DBG_WANPIPE_COMMON("WAN_NETIF_WAKE_QUEUE: file: %s, line: %d\n", __FILE__, __LINE__)
#define WAN_NETIF_QUEUE_STOPPED(a)	DBG_WANPIPE_COMMON("WAN_NETIF_QUEUE_STOPPED: file: %s, line: %d\n", __FILE__, __LINE__)
*/
#define WAN_NETIF_WAKE_QUEUE(a)		0
#define WAN_NETIF_QUEUE_STOPPED(a)	0

#define WAN_NETIF_START_QUEUE(dev)	\
	DBG_NOT_IMPL(("WAN_NETIF_START_QUEUE()\n"))

#define WAN_NETIF_CARRIER_ON	\
	DBG_NOT_IMPL(("WAN_NETIF_CARRIER_ON()\n"))

#define WAN_NETIF_CARRIER_OFF	\
	DBG_NOT_IMPL(("WAN_NETIF_CARRIER_OFF()\n"))

#define WAN_NETIF_STOP_QUEUE	\
	DBG_NOT_IMPL(("WAN_NETIF_STOP_QUEUE()\n"))

#else
# error "Undefined WAN_NETIF_x macros!"
#endif

#if defined(__LINUX__)

# define WAN_SPIN_LOCK_INIT(SpinLock)		\
		spin_lock_init(&((SpinLock)->slock))
# define WAN_SPIN_LOCK_IRQSAVE(SpinLock)	\
		spin_lock_irqsave(&((SpinLock)->slock), (SpinLock)->flags)
# define WAN_SPIN_UNLOCK_IRQSAVE(SpinLock)	\
		spin_unlock_irqrestore(&((SpinLock)->slock), (SpinLock)->flags)

# define WAN_RWLOCK_INIT(rwlock_ptr) \
		rwlock_ptr->rwlock = RW_LOCK_UNLOCKED;
	
#elif defined(__FreeBSD__) || defined(__OpenBSD__)

# define WAN_SPIN_LOCK_INIT(SpinLock)	(SpinLock)->slock = 0
# define WAN_SPIN_LOCK_IRQSAVE(SpinLock)	\
		(SpinLock)->slock = splimp();
# define WAN_SPIN_UNLOCK_IRQSAVE(SpinLock)	\
		splx((SpinLock)->slock);

#elif defined(__WINDOWS__)

# define WAN_SPIN_LOCK_INIT(pSpinLock)		\
{											\
	int rc;									\
	VERIFY_PASSIVE_IRQL(rc);				\
	if(rc == 0){							\
		KeInitializeSpinLock(pSpinLock);	\
	}										\
}

///////////////////////////////////////////////////////
//for use at IRQL <= DISPATCH_LEVEL
# define WAN_SPIN_LOCK_IRQSAVE(pSpinLock)		\
{												\
	int rc=SILENT;								\
	VERIFY_DISPATCH_IRQL(rc);					\
	if(rc == 0){								\
		KeAcquireSpinLock(pSpinLock, &old_IRQL);\
	}											\
}

# define WAN_SPIN_UNLOCK_IRQSAVE(pSpinLock)	\
	KeReleaseSpinLock(pSpinLock, old_IRQL);

//for use at IRQL == DISPATCH_LEVEL - more efficient
//when known to run in DPC
# define WAN_SPIN_LOCK_DPC(pSpinLock)			\
{												\
	int rc;										\
	VERIFY_IRQL_EQUAL_DISPATCH(rc);				\
	if(rc == 0){								\
		KeAcquireSpinLockAtDpcLevel(pSpinLock);	\
	}											\
}

# define WAN_SPIN_UNLOCK_DPC(pSpinLock)	\
	KeReleaseSpinLockFromDpcLevel(pSpinLock);
///////////////////////////////////////////////////////

#else
# error "Undefined WAN_SPIN_LOCK_x macros!"
#endif

#if defined(__LINUX__)
# define WAN_BPF_REPORT(dev,m)
#elif defined(__FreeBSD__)
# if (__FreeBSD_version > 500000)
#  define WAN_BPF_REPORT(dev,m)	bpf_mtap((dev)->if_bpf, (m))
# else
#  define WAN_BPF_REPORT(dev,m)	bpf_mtap((dev), (m))
# endif
#elif defined(__OpenBSD__)
#  define WAN_BPF_REPORT(dev,m)	bpf_mtap((dev)->if_bpf, (m));
#elif defined(__WINDOWS__)
#else
# error "Undefined WAN_BPF_REPORT macro!"
#endif

#if defined (__LINUX__)

# define __WAN_PUT(str)	atomic_dec(&(str)->refcnt)
# define WAN_PUT(str)	if (atomic_dec_and_test(&(str)->refcnt)){ \
		       				wan_kfree(str); 		\
                       	} 
# define WAN_HOLD(str)	atomic_inc(&(str)->refcnt)
#elif defined(__FreeBSD__)
# define __WAN_PUT(str)	atomic_dec(&(str)->refcnt)
# define WAN_PUT(str)	atomic_dec(&str->refcnt);	\
			if (str->refcnt){		\
		       		WAN_FREE(str);		\
                       	} 
# define WAN_HOLD(str)	atomic_inc(&(str)->refcnt)
#elif defined(__OpenBSD__)
# define __WAN_PUT(str)	str->refcnt--
# define WAN_PUT(str)	str->refcnt--;	\
			if (str->refcnt){		\
		       		WAN_FREE(str);		\
                       	} 
# define WAN_HOLD(str)	str->refcnt++

#elif defined(__WINDOWS__)

# define WAN_DEV_PUT(dev)
# define WAN_DEV_HOLD(dev)

# define __WAN_PUT(str)	str->refcnt--

# define WAN_PUT(str)	str->refcnt--;	\
				if(str->refcnt){		\
		       		WAN_FREE(str);		\
               	} 
# define WAN_HOLD(str)	str->refcnt++

#else
# warning "Undefined WAN_HOLD/WAN_PUT macro!"
#endif

#if defined(__FreeBSD__) || defined(__OpenBSD__)
# ifdef ENABLE_SPPP
#  define WAN_SPPP_ENABLED		1
#  define WAN_SPPP_ATTACH(ifp)		sppp_attach(ifp)
#  define WAN_SPPP_DETACH(ifp)		sppp_detach(ifp)
#  define WAN_SPPP_FLUSH(ifp)		sppp_flush(ifp)
#  define WAN_SPPP_PICK(ifp)		sppp_pick(ifp)
#  define WAN_SPPP_DEQUEUE(ifp)		sppp_dequeue(ifp)
#  define WAN_SPPP_ISEMPTY(ifp)		sppp_isempty(ifp)
#  define WAN_SPPP_INPUT(ifp,skb)	sppp_input(ifp,skb)
#  define WAN_SPPP_IOCTL(ifp,cmd,data)	sppp_ioctl(ifp,cmd,data);
# else
#  define WAN_SPPP_ENABLED		0
#  define WAN_SPPP_ATTACH(ifp)		
#  define WAN_SPPP_DETACH(ifp)		
#  define WAN_SPPP_FLUSH(ifp)		
#  define WAN_SPPP_PICK(ifp)		NULL
#  define WAN_SPPP_DEQUEUE(ifp)		NULL
#  define WAN_SPPP_ISEMPTY(ifp)		0
#  define WAN_SPPP_INPUT(ifp,skb)	
#  define WAN_SPPP_IOCTL(ifp,cmd,data)	-EOPNOTSUPP
# endif
#elif defined(__LINUX__)
# define WAN_SPPP_ENABLED		1
# define WAN_SPPP_IOCTL(ifp,cmd,data)	-EOPNOTSUPP
#elif defined(__WINDOWS__)
#else
# error "Undefined WAN_SPPP_x macros!"
#endif

/*
*****************************************************************************
**			T Y P E D E F S				
*****************************************************************************
*/
/*
#if defined(__WINDOWS__)
typedef struct {
	netdevice_t*	dev;
	unsigned char	state;
	unsigned char	usedby;

	void		*card;	

	void* sk;

} wanpipe_common_t;
#endif
*/

#if defined(__KERNEL__)

#define WP_TDM_API_MAX_LEN	1024

typedef struct _netdevice_t
{
	unsigned char	device_type; // must be first member of the structure!!

	unsigned char flags;
	unsigned int mtu;
	unsigned int hard_header_len;
	unsigned int tx_queue_len;
	unsigned int irq;
	unsigned int dma;
	unsigned int base_addr;

	unsigned int mem_start; 
	unsigned int mem_end;

	char name[WAN_IFNAME_SZ+1];

	void * priv;//pointer to the private area
	int (*init)(void* dev);
	int (*open)(void* dev);
	int (*stop)(void* dev);

	void * hard_header;
	void * rebuild_header;

	int (*hard_start_xmit)(void* skb, void* dev);
	void* (*get_stats)(void* dev);

	void	 (*tx_timeout) (void* netdev);

	int (*do_ioctl)(void* dev, void* ifr, int cmd);

	int (*udp_mgmt)(void* card, void* net_dev, void* priv_area);

	wanif_conf_t	wanif_conf;	//configuration of the logical interface.
	int				i_encapsulation_type;//one of: WPLIP_RAW, WPLIP_IP

	//////////////////////////////////////////////////////////////
	//Rx related members
	DATA_QUEUE*		rx_queue;	//allocated on driver startup

	KSPIN_LOCK		spinlock_rx_irp;	//protects rx IRP
	PIRP			pendingRxIrp;

	KTIMER			NoRxDataTimer;
	KDPC			NoRxDataTimerDpcObject;
	LARGE_INTEGER	RxTimeoutDueTime;

    PUCHAR	rx_userbfr;
	ULONG	rx_userbfr_len;

	//////////////////////////////////////////////////////////////
	//Tx related members
	DATA_QUEUE*		tx_queue;	//allocated on driver startup, used ONLY at hardware level
	
	KSPIN_LOCK		spinlock_tx_irp;	//protects tx IRP
	PIRP			pendingTxIrp;

	KTIMER			NoTxInterruptTimer;
	KDPC			NoTxInterruptDataTimerDpcObject;
	LARGE_INTEGER	NoTxInterruptTimeoutDueTime;

    PUCHAR	tx_userbfr;
	ULONG	tx_userbfr_len;

	//////////////////////////////////////////////////////////////

	void * sdla_card;			//pointer back to 'parent' sdla card
	UNICODE_STRING	linkString;	//device name accessible from user mode
	PDEVICE_OBJECT	fdo;

	int	open_handle_counter;

	void* lip_dev;	//pointer to LIP layer interface (i.g. DLCI).
					//used to pass data for transmission.
	//////////////////////////////////////////////////////////////
	//DMA stuff
	PDMA_ADAPTER		AdapterObject;				// DMA adapter object
	ULONG				nMapRegisters;				// maximum # mapping registers
	PHYSICAL_ADDRESS	txPhysicalLogicalAddress;
	PHYSICAL_ADDRESS	rxPhysicalLogicalAddress;

	void* rx_dma_buf;
	void* tx_dma_buf;

	///////////////////////////////////////////////////////////////
	//initialization flag
	char init_done;

	///////////////////////////////////////////////////////////////
	//UDP and IOCTL Management (WANPIPEMON) related stuff
	KSPIN_LOCK	spinlock_ioctl_management;
	PIRP		pending_mgmt_irp;
    PUCHAR		mgmt_userbfr;
	ULONG		mgmt_userbfr_len;

	DATA_QUEUE*		trace_queue;//allocated on driver startup, used ONLY at hardware level

	KTIMER			no_mgmt_data_timer;
	KDPC			no_mgmt_data_dpc_object;
	LARGE_INTEGER	no_mgmt_data_timeout_due_time;

	///////////////////////////////////////////////////////////////
	//these needed to pass received data to driver above:
	//
	//pointer to the wanpipe interface, passed when calling 'receive_indicate()'
	void* wanpipe_if;
	//pointer to receive function, called by sdladrv.sys 
	void (*receive_indicate)(void* card, RX_DATA_STRUCT* rx_data);

	//to indicate to network layer we've completed TX and we can TX again.
	void (*tx_complete_indicate)(void* card);

	//to indicate to layer above we've completed a Management call.
	int (*management_complete)(	void* p_netdevice);

	RX_DATA_STRUCT rx_data;	//Temp buffer for Wanpipe above this interface.
							//used in both sdladrv.sys and sprotocol.sys.
							//Also used by TDMV API for temporary storage
							//for RX Codec operations.

	struct net_device_stats	if_stats;

	void* trace_info;//points to wan_trace_t in chan pointed by 'priv'

	///////////////////////////////////////////////////////////////
	//A104 additions
	unsigned int watchdog_timeo;
	unsigned int trans_start;

	///////////////////////////////////////////////////////////////
	unsigned char (*set_tx_idle_data_in_priv)(void *p_netdevice);

	unsigned int interface_number;	//zero-based number. used by A200 analog code
									//to find out the 'line number'

	//////////////////////////////////////////////////////////////
	//API Poll related members
	//DATA_QUEUE*	api_poll_queue;	//allocated on driver startup
	API_POLL_STRUCT	api_poll_data;

	KSPIN_LOCK		spinlock_api_poll_irp;	//protects API poll IRP
	PIRP			pendingApiPollIrp;

	KTIMER			NoApiPollDataTimer;
	KDPC			NoApiPollDataTimerDpcObject;
	LARGE_INTEGER	ApiPollTimeoutDueTime;

    PUCHAR			api_poll_userbfr;
	ULONG			api_poll_userbfr_len;

	void	*wp_tdm_api_dev_ptr;

	TX_DATA_STRUCT codec_tx_data;	//Used by TDMV API for temporary storage
									//for TX Codec operations.

	char	rx_user_period_data[WP_TDM_API_MAX_LEN];
	unsigned int rx_user_period_data_length;

	char	tx_user_period_data[WP_TDM_API_MAX_LEN];
	unsigned int tx_user_period_data_length;

}netdevice_t;


/*
*****************************************************************************
**		F U N C T I O N   P R O T O T Y P E S				
*****************************************************************************
*/
char*		wanpipe_get_state_string (void*);
void 		wanpipe_set_state (void*, int);
char 		wanpipe_get_state (void*);
void		wanpipe_card_lock_irq (void *,unsigned long *);
void		wanpipe_card_unlock_irq (void *,unsigned long *);
void		wanpipe_set_baud(void*card,unsigned int baud);
unsigned long 	wan_get_ip_addr   (void*, int);
int		wan_udp_pkt_type  (void*, caddr_t);
int		wan_reply_udp	  (void*, unsigned char*, unsigned int);
void 		wanpipe_debug_timer_init(void*);
#if defined(__FreeBSD__) || defined(__OpenBSD__)
void		wanpipe_debugging (void* data, int pending);
#else
void		wanpipe_debugging (unsigned long data);
#endif

extern int sdla_bus_write_4(void* phw, unsigned int offset, u32);
extern int sdla_bus_read_4(void* phw, unsigned int offset, u32*);


/*
*****************************************************************************
**		I N L I N E   F U N C T I O N S				
*****************************************************************************
*/

/******************* WANPIPE VIRT<->BUS SPACE FUNCTION ******************/
/*
** wan_virt2bus
*/
static __inline unsigned long wan_virt2bus(unsigned long* ptr)
{
#if defined(__LINUX__)
	return virt_to_bus(ptr);
#elif defined(__FreeBSD__)
	return vtophys((vm_offset_t)ptr);
#elif defined(__OpenBSD__)
	return vtophys((vaddr_t)ptr);
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_virt2bus()\n"));
#else
# error "wan_virt2bus() function is not supported yet!"
#endif
}

/*
** wan_bus2virt
*/
static __inline unsigned long* wan_bus2virt(unsigned long virt_addr)
{
#if defined(__LINUX__)
	return bus_to_virt(virt_addr);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return (unsigned long*)virt_addr;
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_bus2virt()\n"));
#else
# error "wan_bus2virt() function is not supported yet!"
#endif
}


/******************* WANPIPE DMA FUNCTION ******************/

/*
** wan_dma_alloc
*/
static __inline int
wan_dma_alloc(void* hw, wan_dma_descr_t* dma_descr)
{
	int	err = 0;
#if defined(__FreeBSD__)
	err = bus_dma_tag_create(/*parent*/NULL, 
				  /*alignemnt*/1, 
				  /*boundary*/0,
			          /*lowaddr*/BUS_SPACE_MAXADDR_32BIT,
			          /*highaddr*/BUS_SPACE_MAXADDR,
			          /*filter*/NULL, /*filterarg*/NULL,
			          /*maxsize*/dma_descr->max_length,
			          /*nsegments*/BUS_SPACE_UNRESTRICTED,
			          /*maxsegsz*/BUS_SPACE_MAXSIZE_32BIT,
			          /*flags*/0, 
				  (bus_dma_tag_t*)&dma_descr->dmat);
	if (err){
		DEBUG_EVENT("Failed create DMA tag (size=%ld)!\n",
					dma_descr->max_length);
		dma_descr->max_length = 0;
		return err;
	}
	err = bus_dmamem_alloc(dma_descr->dmat, 
			       (void**)&dma_descr->vAddr, 
			       BUS_DMA_NOWAIT, 
			       &dma_descr->dmamap);
	if (err){
		DEBUG_EVENT("Failed allocate DMA (size=%ld)!\n",
					dma_descr->max_length);
		bus_dma_tag_destroy(dma_descr->dmat);
		dma_descr->max_length = 0;
		return err;
	}
#elif defined(__OpenBSD__)
	err = bus_dmamem_alloc(dma_descr->dmat, 	/* dma tag */
				dma_descr->max_length, 	/* size */
				PAGE_SIZE, 		/* alignment */
				0,			/* boundary */
				&dma_descr->dmaseg,	/* serments */
				1,			/* num of segments */
				&dma_descr->rsegs,	/* R num of segments */
				BUS_DMA_NOWAIT);	
	if (err){
		DEBUG_EVENT("Failed allocate DMA segment (size=%ld)!\n",
					dma_descr->max_length);
		dma_descr->max_length = 0;
		return err;
	}
	err = bus_dmamem_map(dma_descr->dmat,		/* dma tag */
				&dma_descr->dmaseg,	/* segments */
				dma_descr->rsegs,	/* return num of segments */
				dma_descr->max_length, 	/* size */
				(caddr_t*)&dma_descr->vAddr,	/* kernel virtual address */
				BUS_DMA_NOWAIT);
	if (err){
		DEBUG_EVENT("Failed map DMA segment (size=%ld)!\n",
					dma_descr->max_length);
		dma_descr->max_length = 0;
		bus_dmamem_free(dma_descr->dmat, &dma_descr->dmaseg, dma_descr->rsegs);
		return err;
	}		
#elif defined(__LINUX__)
	dma_descr->vAddr = pci_alloc_consistent(NULL,dma_descr->max_length,(dma_addr_t *)&dma_descr->pAddr); 
	if (dma_descr->vAddr == NULL){
		err = -ENOMEM;
	}
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_dma_alloc()\n"));
	return -EINVAL;
#else
# error "wan_dma_alloc() function is not supported yet!"
#endif
	return err;
}

/*
** wan_dma_free
*/
static __inline int
wan_dma_free(void* hw, wan_dma_descr_t* dma_descr)
{
#if defined(__FreeBSD__)
	bus_dmamem_free(dma_descr->dmat, dma_descr->vAddr, dma_descr->dmamap);
	return bus_dma_tag_destroy(dma_descr->dmat);
#elif defined(__OpenBSD__)
	bus_dmamem_unmap(dma_descr->dmat, (caddr_t)dma_descr->vAddr, dma_descr->max_length);
	bus_dmamem_free(dma_descr->dmat, &dma_descr->dmaseg, dma_descr->rsegs);
#elif defined(__LINUX__)

	DEBUG_TEST("Freeing Pages 0x%p len=%li  order=%i\n",
		dma_descr->vAddr,
		dma_descr->max_length,
		get_order(dma_descr->max_length));

	pci_free_consistent(NULL, dma_descr->max_length,dma_descr->vAddr,dma_descr->pAddr);
	dma_descr->vAddr = NULL;
	dma_descr->pAddr = 0;
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_dma_free()\n"));
	return -EINVAL;
#else
# error "wan_dma_free() function is not supported yet!"
#endif
	return 0;
}



/********************** WANPIPE KERNEL BUFFER **************************/

/*
**************************************************************************
**			 WANPIPE NETWORK INTERFACE 			**
**************************************************************************
*/
static __inline int wan_netif_init(netdevice_t* dev, char* ifname, int ifunit)
{

#if defined(__FreeBSD__)
	int		len = 0;

        while((ifname[len] != '\0') && (len <= strlen(ifname))){
		if ((ifname[len] >= '0') && (ifname[len] <= '9')){
			break;
		}
		len++;    
	}
	dev->if_unit	= ifunit;
	dev->if_name	= malloc(len+1, M_DEVBUF, M_NOWAIT);
	if (dev->if_name == NULL){
		return -ENOMEM;
	}
	memset(dev->if_name, 0, len+1);
	bcopy(ifname, dev->if_name, len);
	dev->if_name[len] = '\0';
	IFQ_SET_MAXLEN(&dev->if_snd, ifqmaxlen);
#elif defined(__OpenBSD__)
	if (strlen(ifname) >= IFNAMSIZ){
		return -ENOMEM;
	}
    	bcopy(ifname, dev->if_xname, strlen(ifname));
	IFQ_SET_MAXLEN(&dev->if_snd, IFQ_MAXLEN);
#elif defined(__LINUX__)
# if  (LINUX_VERSION_CODE < KERNEL_VERSION(2,3,43))
	dev->name = ifname;
# else	
	strcpy(dev->name, ifname);
# endif

#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_netif_init()\n"));
#else
# error "wan_netif_init() function is not supported yet!"
#endif
	return 0;
}


static __inline int wan_netif_del(netdevice_t* dev)
{
#if defined(__FreeBSD__)
	dev->if_init = NULL;
# if (__FreeBSD_version > 400000)
	/* Free interface name (only for FreeBSD-4.0) */
	free(dev->if_name, M_DEVBUF);
# endif
#elif defined(__OpenBSD__)
	/* Do nothing */
#elif defined(__LINUX__)
	/* Do nothing */
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_netif_del()\n"));
#else
# error "wan_netif_del() function is not supported yet!"
#endif
	return 0;
}

#define IFNAMSIZ 255

static __inline char* wan_netif_name(netdevice_t* dev)
{
	static char ifname[IFNAMSIZ+1];
#if defined(__LINUX__)
	strcpy(ifname, dev->name);
#elif defined(__FreeBSD__)
	sprintf(ifname, "%s%d", dev->if_name, dev->if_unit);
#elif defined(__OpenBSD__)
	sprintf(ifname, "%s", dev->if_xname);
#elif defined(__WINDOWS__)
	strcpy(ifname, dev->name);
#else
# error "wan_get_ifname() function is not supported yet!"
#endif
	return ifname;
}

static __inline void* wan_netif_priv(netdevice_t* dev)
{
	if (dev == NULL){
		return NULL;
	}
#if defined(__LINUX__)
	return dev->priv;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return dev->if_softc;
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_netif_priv()\n"));
	//return dev->priv;
	return NULL;
#else
# error "wan_netif_priv() function is not supported yet!"
#endif
}

static __inline short wan_netif_flags(netdevice_t* dev)
{
#if defined(__LINUX__)
	return dev->flags;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return dev->if_flags;
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_netif_flags()\n"));
	return 0;
#else
# error "wan_netif_flags() function is not supported yet!"
#endif
}

static __inline int wan_netif_mcount(netdevice_t* dev)
{
#if defined(__LINUX__)
	return dev->mc_count;
#elif defined(__FreeBSD__)
	return dev->if_amcount;
#elif defined(__OpenBSD__)
	return 0;
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_netif_mcount()\n"));
	return 0;
#else
# error "wan_netif_mcount() function is not supported yet!"
#endif
}


static __inline void wan_bpf_report(netdevice_t* dev, void* pkt, int flag)
{

#if defined(__LINUX__)
	/* Do nothing */
#elif defined(__FreeBSD__)
	if (dev->if_bpf != NULL){ /* BF-0002 */ 
		WAN_BPF_REPORT(dev, pkt);
	}
#elif defined(__OpenBSD__)
	if (dev->if_bpf != NULL){ /* BF-0002 */ 
		if (flag){
			struct mbuf m0;
			u_int32_t af = AF_INET;
			m0.m_next = pkt;
			m0.m_len = 4;
			m0.m_data = (char*)&af;
			bpf_mtap(dev->if_bpf, &m0);
			WAN_BPF_REPORT(dev, &m0);
		}else{
			WAN_BPF_REPORT(dev, pkt);
		}
	}
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wan_bpf_report()\n"));
#else
# error "wan_bpf_report() function is not supported yet!"
#endif
}

////////////////////////////////////////////////////////////////////////////////
//This function is different from WAN_SPIN_LOCK_IRQSAVE(),
//which only raises priority to DISPATCH_LEVEL.
//It disables ANY interrupts from coming in.
//Check that NOT already at Interrupt Or PROFILE_LEVEL IRQL!!!
//
static __inline void wan_spin_lock_irq(void *old_irql, unsigned long *not_used)
{
#if defined(__LINUX__)
	spin_lock_irqsave(lock,*flag);	
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	*((int*)lock) = splimp();
#elif defined(__WINDOWS__)
	KIRQL current_irql = KeGetCurrentIrql();
	int rc=SILENT;

	//////////////////////////////////////////////////////////////////////////////
	*(KIRQL*)old_irql = current_irql;	//This is done in order for subsequent
										//wan_spin_unlock_irq() calls to work.
										//DO IT EVEN IF THIS FUNCTION FAILED!!
	//////////////////////////////////////////////////////////////////////////////

	VERIFY_DISPATCH_IRQL(rc)
	if(rc){
		DBG_WANPIPE_COMMON("wan_spin_lock_irq(): Warning: not raising IRQL! current irql: %d, PROFILE_LEVEL: %d\n",
			current_irql, PROFILE_LEVEL);
		return;
	}

	KeRaiseIrql(PROFILE_LEVEL, (KIRQL*)old_irql);
#else
# warning "wan_spin_lock_irq() function is not supported yet!"
#endif	
}

//Returns priority to the original one. Must follow wan_spin_lock_irq() call!!
static __inline void wan_spin_unlock_irq(void *new_irql, unsigned long *not_used)
{
#if defined(__LINUX__)
	spin_unlock_irqrestore(lock,*flag);	
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	splx(*(int*)lock);
#elif defined(__WINDOWS__)
	KIRQL current_irql = KeGetCurrentIrql();

	if(*(KIRQL*)new_irql < current_irql){

		KeLowerIrql(*(KIRQL*)new_irql);

	}else{
		DBG_WANPIPE_COMMON("wan_spin_unlock_irq(): Warning: not lowering IRQL! new_irql: %d, current_irql: %d\n",
			*(KIRQL*)new_irql, current_irql);
	}
#else
# warning "wan_spin_unlock_irq() function is not supported yet!"
#endif	
}

////////////////////////////////////////////////////////////////////////////////
//Get a 'regular' spin lock (priority raised to DISPATCH_LEVEL).
static __inline void wan_spin_lock(void *lock)
{
#if defined(__LINUX__)
	spin_lock(lock);	
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	*((int*)lock) = splimp();
#elif defined(__WINDOWS__)
	wan_spinlock_t* wan_spinlock = (wan_spinlock_t*)lock;
	KIRQL current_irql = KeGetCurrentIrql();
	int rc=SILENT;

	VERIFY_DISPATCH_IRQL(rc)
	if(rc){
		DBG_WANPIPE_COMMON("%s(): Warning: invalid current irql: %d!\n",
			__FUNCTION__, current_irql);
		return;
	}

	DBG_WANPIPE_COMMON("%s()\n", __FUNCTION__);

	KeAcquireSpinLock(&wan_spinlock->slock, &wan_spinlock->flags);
#else
# warning "wan_spin_lock() function is not supported yet!"
#endif	
}

//Release the 'regular' spin lock.
static __inline void wan_spin_unlock(void *lock)
{
#if defined(__LINUX__)
	spin_unlock(lock);	
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	splx(*(int*)lock);
#elif defined(__WINDOWS__)
	wan_spinlock_t* wan_spinlock = (wan_spinlock_t*)lock;
	KIRQL current_irql = KeGetCurrentIrql();
	int rc=SILENT;

	VERIFY_DISPATCH_IRQL(rc)
	if(rc){
		DBG_WANPIPE_COMMON("%s(): Warning: invalid current irql: %d!\n",
			__FUNCTION__, current_irql);
		return;
	}

	DBG_WANPIPE_COMMON("%s()\n", __FUNCTION__);

	KeReleaseSpinLock(&wan_spinlock->slock, (KIRQL)wan_spinlock->flags);
#else
# warning "wan_spin_unlock() function is not supported yet!"
#endif	
}

////////////////////////////////////////////////////////////////////////////////
//bitswap each byte in a buffer
extern unsigned char wp_brt[256]; 
static __inline void wan_skb_reverse(unsigned char *data, int len)
{
	int i;
	for (i=0; i < len; i++){
		data[i]=wp_brt[data[i]];
	}
}

////////////////////////////////////////////////////////////////////////////////
#if 0
static __inline void wan_read_rw_lock(void *lock)
{
#if defined(__LINUX__)
	read_lock(((rwlock_t*)lock));
#else
# warning "wan_read_rw_lock() function is not supported yet!"
#endif
}

static __inline void wan_read_rw_unlock(void *lock)
{
#if defined(__LINUX__)
	read_unlock(((rwlock_t*)lock));
#else
# warning "wan_read_rw_unlock() function is not supported yet!"
#endif
}

static __inline void wan_write_rw_lock_irq(void *lock, unsigned long *flag)
{
#if defined(__LINUX__)
	write_lock_irqsave(((rwlock_t*)lock),flag);
#else
# warning "wan_read_rw_lock() function is not supported yet!"
#endif
}

static __inline void wan_write_rw_unlock_irq(void *lock, unsigned long *flag)
{
#if defined(__LINUX__)
	write_unlock_irqrestore(((rwlock_t*)lock),flag);
#else
# warning "wan_read_rw_unlock() function is not supported yet!"
#endif
}
#endif

#define wan_clear_bit(a,b)  	    clear_bit((a),(unsigned long*)(b))
#define wan_set_bit(a,b)    	    set_bit((a),(unsigned long*)(b))
#define wan_test_bit(a,b)   	    test_bit((a),(unsigned long*)(b))
#define wan_test_and_set_bit(a,b)	test_and_set_bit((a),(unsigned long*)(b))

#define start_net_queue
#define stop_net_queue

#define PCI_DMA_TODEVICE	1
#define PCI_DMA_FROMDEVICE	2

#define IFF_UP	1

static __inline void pci_unmap_single(void* a, unsigned long b, unsigned long c, int d)
{
	DBG_NOT_IMPL(("pci_unmap_single()\n"));
}

static __inline unsigned long pci_map_single(void* a, void* b, unsigned long c, int d)
{
	DBG_NOT_IMPL(("pci_map_single()\n"));
	return 0;
}

static __inline void wan_capture_trace_packet(void* card, void* trace_info, void* skb, int direction)
{
	DBG_NOT_IMPL(("wan_capture_trace_packet()\n"));
}

static __inline int net_ratelimit()
{
	//DBG_WANPIPE_COMMON("net_ratelimit()\n");
	return 1;
}

#define WAN_NET_RATELIMIT	net_ratelimit

#define	is_digit(ch) (((ch)>=(unsigned)'0'&&(ch)<=(unsigned)'9')?1:0)

/*============================================================================
 * Convert decimal string to unsigned integer.
 * If len != 0 then only 'len' characters of the string are converted.
 */
static unsigned int dec_to_uint (unsigned char* str, int len)
{
	unsigned val;

	if (!len) len = strlen(str);
	for (val = 0; len && is_digit(*str); ++str, --len)
		val = (val * 10) + (*str - (unsigned)'0')
	;
	return val;
}

static __inline int wan_netif_set_ticks(netdevice_t* dev, unsigned long ticks)
{
#if defined(__LINUX__)
	dev->trans_start = ticks;
#elif defined(__WINDOWS__)
	dev->trans_start = ticks;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#else
# error "wan_netif_set_ticks() function is not supported yet!"
#endif
	return 0;
}


static __inline void wan_read_bus_4(void *phw, void *virt, int offset, unsigned int *value)
{
#if defined(__LINUX__)
	*value = wp_readl((unsigned char*)virt + offset); 
#else
    sdla_bus_read_4(phw,offset,value);
#endif	
}

static __inline void wan_write_bus_4(void *phw, void *virt, int offset, unsigned int value)
{
#if defined(__LINUX__)
	wp_writel(value,(u8*)virt + offset);
#else
    sdla_bus_write_4(phw,offset,value);
#endif	
}   


#endif//__KERNEL__

#endif	/* __WANPIPE_COMMON_H */
