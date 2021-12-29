/*****************************************************************************
* wanpipe_abstr.h WANPIPE(tm) Kernel Abstraction Layer.
*
* Authors: 	Nenad Corbic <ncorbic@sangoma.com>
*		Alex Feldman <al.feldman@sangoma.com
*
* Copyright:	(c) 2003 Sangoma Technologies Inc.
*
* ============================================================================
* Jan 20, 2003  Nenad Corbic	Initial version
* ============================================================================
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2, or (at your option)
* any later version.
*
*/

#ifndef _WANPIPE_ABSTR_H
#define _WANPIPE_ABSTR_H

#include <little_endian.h>
#include <wanpipe_cfg.h>
#include <wanpipe_common.h>

#include <irql_check.h>

#define DBG_WANPIPE_ABSTR	DbgPrint
//#define DBG_WANPIPE_ABSTR

#define MYEXTERN

/////////////////////////////////////////////////////////////
//Prototypes of unabstructed functions.
//Must be only used internally by 'wanpipe_abstr.h'
int wpabs_skb_headlen(const struct sk_buff *skb);

//End of internal functions prototypes
/////////////////////////////////////////////////////////////

MYEXTERN unsigned char* 	wpabs_skb_data(void* skb);
MYEXTERN unsigned char* 	wpabs_skb_tail(void* skb);

static __inline unsigned short wpabs_skb_len(void* skb);
static __inline void *wpabs_skb_dequeue(void* list);

MYEXTERN void		wpabs_skb_free(void* skb);

MYEXTERN unsigned char* 	wpabs_skb_pull(struct sk_buff* skb, unsigned int len);
MYEXTERN unsigned char* 	wpabs_skb_put(struct sk_buff* skb, int len);


MYEXTERN void		wpabs_skb_trim(struct sk_buff* skb, unsigned int len);
MYEXTERN int		wpabs_skb_tailroom(const struct sk_buff *skb);
MYEXTERN int		wpabs_skb_headroom(void *skb);
MYEXTERN void		wpabs_skb_init(void* skb, unsigned int len);
MYEXTERN int		wpabs_skb_queue_len(void *queue);
MYEXTERN void		wpabs_skb_queue_tail(void *queue, void *skb);

MYEXTERN void		wpabs_skb_set_dev(void *skb, void *dev);
MYEXTERN void		wpabs_skb_set_raw(void *skb);
MYEXTERN void		wpabs_skb_set_protocol(void *skb, unsigned int prot);

MYEXTERN int 		wpabs_netif_queue_stopped(void*);
MYEXTERN int		wpabs_netif_dev_up(void*);
MYEXTERN void 		wpabs_netif_wake_queue(void* dev);

MYEXTERN void*		wpabs_timer_alloc(void);
MYEXTERN void 		wpabs_init_timer(void*, void*, unsigned long);
MYEXTERN void 		wpabs_del_timer(void*);
MYEXTERN void 		wpabs_add_timer(void*,unsigned long);

MYEXTERN void*		wpabs_malloc(int);
MYEXTERN void		wpabs_free(void*);

MYEXTERN void* 		wpabs_dma_alloc(void*, unsigned long);
MYEXTERN int 		wpabs_dma_free(void*, void*);
MYEXTERN unsigned long*	wpabs_dma_get_vaddr(void*, void*);
MYEXTERN unsigned long	wpabs_dma_get_paddr(void*, void*);

MYEXTERN unsigned long*	wpabs_bus2virt(unsigned long phys_addr);
MYEXTERN unsigned long	wpabs_virt2bus(unsigned long* virt_addr);

MYEXTERN unsigned char	wpabs_bus_read_1(void*, int);      
MYEXTERN unsigned long	wpabs_bus_read_4(void*, int);      
MYEXTERN void		wpabs_bus_write_1(void*, int, unsigned char);      
MYEXTERN void		wpabs_bus_write_4(void*, int, unsigned long);      

MYEXTERN void 		wpabs_udelay(unsigned long microsecs);

MYEXTERN void* 		wpabs_spinlock_alloc(void);
MYEXTERN void 		wpabs_spinlock_free(void*);
MYEXTERN void 		wpabs_spin_lock_irqsave(void*);
MYEXTERN void 		wpabs_spin_unlock_irqrestore(void*);
MYEXTERN void 		wpabs_spin_lock_init(void*);

MYEXTERN void 		wpabs_rwlock_init (void*);
MYEXTERN void 		wpabs_read_rw_lock(void*);
MYEXTERN void 		wpabs_read_rw_unlock(void*);
MYEXTERN void 		wpabs_write_rw_lock_irq(void*,unsigned long*);
MYEXTERN void		wpabs_write_rw_unlock_irq(void*,unsigned long*);


MYEXTERN void 		wpabs_debug_event(const char * fmt, ...);
MYEXTERN void 		wpabs_debug_init(const char * fmt, ...);
//MYEXTERN void 		wpabs_debug_cfg(const char * fmt, ...);
MYEXTERN void 		wpabs_debug_tx(const char * fmt, ...);
MYEXTERN void 		wpabs_debug_rx(const char * fmt, ...);
MYEXTERN void 		wpabs_debug_isr(const char * fmt, ...);
MYEXTERN void 		wpabs_debug_timer(const char * fmt, ...);

//MYEXTERN int 		wpabs_set_bit(int bit, void *ptr);
//MYEXTERN int 		wpabs_test_bit(int bit, void *ptr);
MYEXTERN int		wpabs_test_and_set_bit(int bit, void *ptr);
//MYEXTERN int 		wpabs_clear_bit(int bit, void *ptr);

MYEXTERN unsigned long 	wpabs_get_systemticks(void);
MYEXTERN unsigned long 	wpabs_get_hz(void);

#if 0
MYEXTERN void		wpabs_lan_rx(void*,void*,unsigned long,unsigned char*, int);
MYEXTERN void		wpabs_tx_complete(void*, int, int);

MYEXTERN void* 		wpabs_ttydriver_alloc(void);
MYEXTERN void 		wpabs_ttydriver_free(void*);
MYEXTERN void* 		wpabs_termios_alloc(void);
MYEXTERN void		wpabs_tty_hangup(void*);

MYEXTERN int 		wpabs_wan_register(	void *, void *, char *,	unsigned char);

MYEXTERN void 		wpabs_wan_unregister(void *, unsigned char);
#endif

MYEXTERN void* 		wpabs_taskq_alloc(void);
MYEXTERN void 		wpabs_taskq_init(void *, void *, void *);
MYEXTERN void 		wpabs_taskq_schedule_event(unsigned int, unsigned long *, void *);

MYEXTERN void 		wpabs_tasklet_kill(void *);
MYEXTERN void 		wpabs_tasklet_end(void *);
MYEXTERN void 		wpabs_tasklet_schedule(void *);
MYEXTERN void 		wpabs_tasklet_init(void *, int , void *, void* );
MYEXTERN void*		wpabs_tasklet_alloc(void);

MYEXTERN void* 		wpabs_memset(void *, int , int);
MYEXTERN void* 		wpabs_strncpy(void *, void* , int);
MYEXTERN void* 		wpabs_memcpy(void *, void* , int);

MYEXTERN void		wpabs_debug_print_skb(void*,char);
 
MYEXTERN void		wpabs_set_baud(void*, unsigned int);
MYEXTERN void		wpabs_set_state(void*, int);
MYEXTERN char		wpabs_get_state(void*);
MYEXTERN unsigned long	wpabs_get_ip_addr(void*, int);

MYEXTERN void 		wpabs_decode_ipaddr(unsigned long, unsigned char *);

MYEXTERN int 		wpabs_detect_prot_header(unsigned char *,int, char*);
//MYEXTERN int		wpabs_net_ratelimit(void);

MYEXTERN void		wpabs_card_lock_irq(void *,unsigned long *);
MYEXTERN void		wpabs_card_unlock_irq(void *,unsigned long *);

#define TRC_INCOMING_FRM              0x00
#define TRC_OUTGOING_FRM              0x01

MYEXTERN int		wpabs_trace_queue_len(void *trace_ptr);
MYEXTERN int		wpabs_tracing_enabled(void*);
MYEXTERN int		wpabs_trace_enqueue(void*, void*);
MYEXTERN int		wpabs_trace_purge(void*);
MYEXTERN void* 		wpabs_trace_info_alloc(void);
MYEXTERN void 		wpabs_trace_info_init(void *trace_ptr, int max_queue);


#pragma pack(1)
typedef struct {
	unsigned char	status;
	unsigned char	data_avail;
	unsigned short	real_length;
	unsigned short	time_stamp;
	unsigned char	data[1];
} wan_trace_pkt_t;
#pragma pack()


struct ifreq{

	char *  ifr_data;
};

struct seq_file{

	int count;
};

typedef struct timeval { 
	long tv_sec;
	long tv_usec;
} timeval;

////////////////////////////////////////////////////////////////
#define MAX_SKB_LEN	4000
#define MIN_SKB_LEN	1

#ifndef	MINIMUM_LIP_HEAD_ROOM
 #define MINIMUM_LIP_HEAD_ROOM	16
#endif

#define wpabs_debug_test	DEBUG_ABSTRACT
#define wpabs_debug_event	DEBUG_ABSTRACT
#define wpabs_debug_cfg		DEBUG_ABSTRACT

#define wpabs_set_bit		wan_set_bit
#define wpabs_clear_bit		wan_clear_bit
#define wpabs_net_ratelimit	net_ratelimit
#define wpabs_test_bit		wan_test_bit

#define wpabs_strncpy		strncpy
#define wpabs_memcpy		RtlCopyMemory
//#define wpabs_memset(dest, fill_char, len)	RtlFillMemory(dest, len, fill_char)
#define wpabs_memset		memset
#define wpabs_strlen		strlen
#define wpabs_memcmp		memcmp

static _inline unsigned long wpabs_get_systemticks()
{
	LARGE_INTEGER	CurrentTime;

	/*
	10 000 000
	System time is a count of 100-nanosecond intervals
	since January 1, 1601. System time is typically
	updated approximately every ten milliseconds. (on Intel 32bit)
	*/
	KeQuerySystemTime(&CurrentTime);

//return (ULONG)(CurrentTime.QuadPart / 10000000); //incremented by second
//return (ULONG)(CurrentTime.QuadPart / 100000);   //-original, incremented by 1/100 of a second
  return (ULONG)(CurrentTime.QuadPart / 10000);    //incremented by 1/1000 (1 millisecond)
}

//Function returns 'Number of system ticks' over
//time period of 1 secod.
//Since wpabs_get_systemticks() operates with a 1/1000 of a second,
//this function has to return 1000 on 32-bit system.
static __inline unsigned long wpabs_get_hz()
{
	return 1000;
}

#define HZ	wpabs_get_hz()

////////////////////////////////////////////////////////////////
static unsigned short wpabs_htons(unsigned short hostshort)
{
	DEBUG_ABSTRACT("wpabs_htons()\n");
	return __constant_htons(hostshort);
}

static unsigned long wpabs_htonl(unsigned long hostlong)
{
	DEBUG_ABSTRACT("wpabs_htonl()\n");
	return __constant_htonl(hostlong);
}

static unsigned short wpabs_ntohs(unsigned short netshort)
{
	DEBUG_ABSTRACT("wpabs_ntohs()\n");
	return __constant_ntohs(netshort);
}

static unsigned long wpabs_ntohl(unsigned long netlong)
{
	DEBUG_ABSTRACT("wpabs_ntohl()\n");
	return __constant_ntohl(netlong);
}

static void* wpabs_malloc(int size)
{
	void* tmp;
	int rc=1;

	DEBUG_HWEC("wpabs_malloc(): %d\n", size);

	VERIFY_DISPATCH_IRQL(rc);
	if(rc){
		DEBUG_EVENT("Invalid IRQL allocating memory!!\n");
		return NULL;
	}

	tmp = ExAllocatePool(NonPagedPool, size);
	if(tmp != NULL){
		RtlZeroMemory(tmp, size);
	}else{
		DEBUG_EVENT("Failed to allocate %d bytes of memory!!\n", size);
	}

	DEBUG_HWEC("%s(): returning: 0x%p\n", __FUNCTION__, tmp);
	return tmp;
}

static void wpabs_free(void *ptr)
{
	int rc=1;

	//DEBUG_ABSTRACT("wpabs_free()\n");
	DEBUG_HWEC("%s(): 0x%p\n", __FUNCTION__, ptr);

	VERIFY_DISPATCH_IRQL(rc);
	if(rc){
		DEBUG_EVENT("Invalid IRQL deallocating memory!!\n");
		return;
	}
	ExFreePool(ptr);
}

//////////////////////////////////////////////////////////////
/*
** wpabs_skb_alloc() - 
**		Allocate kernel buffer with len.
*/
static __inline void* wpabs_skb_alloc(unsigned int len)
{
#if defined(__LINUX__)
#if defined(WAN_DEBUG_MEM)
	struct sk_buff *skb=dev_alloc_skb(len);
	if (skb){
		DEBUG_ADD_MEM(skb->truesize);
	}
	return (void*)skb;
#else
	return (void*)dev_alloc_skb(len);
#endif
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	new_mbuf = NULL;
	MGETHDR(new_mbuf, M_DONTWAIT, MT_DATA);
	if (new_mbuf == NULL)
		return NULL;
	if (dev){
		new_mbuf->m_pkthdr.rcvif	= dev;
	}
	new_mbuf->m_pkthdr.len		= new_mbuf->m_len = 0;
	if ((len+2) > MHLEN){
		MCLGET(new_mbuf, M_DONTWAIT);
		if ((new_mbuf->m_flags & M_EXT) == 0){
			m_freem(new_mbuf);
			return NULL;
		}
	}
	new_mbuf->m_data += 2;
	return (void*)new_mbuf;
#elif defined(__WINDOWS__)
	/**
	 *      dev_alloc_skb - allocate a skbuff
	 *      @length: length to allocate
	 *
	 *      Allocate a new &sk_buff and assign it a usage count of one. The
	 *      buffer has unspecified headroom built in. Users should allocate
	 *      the headroom they think they need without accounting for the
	 *      built in space. The built in space is used for optimisations.
	 *
	 *      %NULL is returned if there is no free memory.
	 */
	struct sk_buff *skb;

	DEBUG_ABSTRACT("wpabs_skb_alloc()\n");

	if(len < MIN_SKB_LEN){
		DEBUG_EVENT("wpabs_skb_alloc(): len of %d is less than MIN_SKB_LEN of %d!!!\n",
			len, MIN_SKB_LEN);
		return NULL;
	}

	if(len > MAX_SKB_LEN){
		DEBUG_EVENT("wpabs_skb_alloc(): len of %d is greater than MAX_SKB_LEN of %d!!!\n",
			len, MAX_SKB_LEN);
		return NULL;
	}

	skb = wpabs_malloc(sizeof(struct sk_buff));
	if(skb == NULL){
		DEBUG_ABSTRACT("Failed to allocate 'skb'!\n");
		return NULL;
	}

	//always allocate additional MINIMUM_LIP_HEAD_ROOM bytes for the header!!
	len += MINIMUM_LIP_HEAD_ROOM;

	skb->head = wpabs_malloc(len);
	if(skb->head == NULL){
		DEBUG_EVENT("Failed to allocate 'skb' data area!'\n");
		wpabs_free(skb);
		return NULL;
	}

    //initialize skb pointers.
	skb->data = skb->head;
	skb->tail = skb->head;
	skb->end  = skb->data + len;

    //initialize skb state
    skb->len = 0;
    skb->cloned = 0;
    skb->data_len = 0;

	skb->protocol = 1;

	return (void*)skb;
#else
# error "wpabs_skb_alloc() function is not supported yet!"
#endif
}

/*
** wpabs_skb_check() - 
**		Check if packet consists from one skb block.
*/
/*
static __inline int wpabs_skb_check(void* skb)
{
#if defined(__LINUX__)
	return 0;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	netskb_t*	m = (netskb_t*)skb;
	if (m->m_pkthdr.len != m->m_len){
		return 1;
	}
	return 0;
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT(("wpabs_skb_check()\n"));
	return 0;
#else
# error "wpabs_skb_check() function is not supported yet!"
#endif
}
*/

/*
** wpabs_skb_copyback() - 
** 	Copy data from a buffer back into the indicated mbuf chain,
** 	starting "off" bytes from the beginning, extending the mbuf
** 	chain if necessary.
*/
/*
static __inline void wpabs_skb_copyback(void* skb, int off, int len, caddr_t cp)
{
#if defined(__LINUX__)
	struct sk_buff* sk = (struct sk_buff*)skb;
	unsigned char* data = NULL;
	if (off == wpabs_skb_len(skb)){
		if (sk->tail + len > sk->end){	
			DEBUG_ABSTRACT("wpabs_skb_copyback: Internal Error (off=%d,len=%d,skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
			return;
		}else{
			data = wpabs_skb_put(skb, len);
			memcpy(data, cp, len);
		}
	}else{
		if (off + len > wpabs_skb_len(skb)){
			data = wpabs_skb_put(skb, len);
			memcpy(data + off, cp, len);
			wpabs_skb_trim(skb, off + len);
		}
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	m = (struct mbuf*)skb;
	caddr_t		data = mtod(m, caddr_t);

	bcopy(cp, &data[off], len); 
	m->m_len 	= off + len;
	m->m_pkthdr.len = off + len;
#elif defined(__WINDOWS__)

	struct sk_buff* sk = (struct sk_buff*)skb;
	unsigned char* data = NULL;

	DEBUG_ABSTRACT("wpabs_skb_copyback()\n");

	if (off == wpabs_skb_len(skb)){
		if (sk->tail + len > sk->end){	
			DEBUG_ABSTRACT("wpabs_skb_copyback: Internal Error (off=%d,len=%d,skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
			return;
		}else{
			data = wpabs_skb_put(skb, len);
			memcpy(data, cp, len);
		}
	}else{
		if (off + len > wpabs_skb_len(skb)){
			data = wpabs_skb_put(skb, len);
			memcpy(data + off, cp, len);
			wpabs_skb_trim(skb, off + len);
		}
	}
#else
# error "wpabs_skb_copyback() function is not supported yet!"
#endif
}
*/

/*
** wpabs_skb_copyback_user() - 
** 	Copy data from a buffer back into the indicated mbuf chain,
** 	starting "off" bytes from the beginning, extending the mbuf
** 	chain if necessary.
**      Data being copied is coming from user space, thus we must
**      use a special function to copy it into kernel space.
*/
/*
static __inline int wpabs_skb_copyback_user(void* skb, int off, int len, caddr_t cp)
{
#if defined(__LINUX__)
	struct sk_buff* sk = (struct sk_buff*)skb;
	unsigned char* data = NULL;
	if (off == wpabs_skb_len(skb)){
		if (sk->tail + len > sk->end){	
			DEBUG_ABSTRACT("wpabs_skb_copyback_user: Internal Error (off=%d,len=%d,skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
			return -EINVAL;
		}else{
			data = wpabs_skb_put(skb, len);
			if (WAN_COPY_FROM_USER(data, cp, len)){
				DEBUG_ABSTRACT("wpabs_skb_copyback_user: Internal Error (off=%d,len=%d,skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
				return -EFAULT;
			}
		}
	}else{
		if (off + len > wpabs_skb_len(skb)){
			data = wpabs_skb_put(skb, len);
			if (WAN_COPY_FROM_USER(data+off, cp, len)){
				DEBUG_ABSTRACT("wpabs_skb_copyback_user: Internal Error (off=%d,len=%d,skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
				return -EFAULT;
			}
			wpabs_skb_trim(skb, off + len);
		}
	}
	return 0;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	m = (struct mbuf*)skb;
	caddr_t		data = mtod(m, caddr_t);

	WAN_COPY_FROM_USER(cp, &data[off], len);
	m->m_len 	= off + len;
	m->m_pkthdr.len = off + len;
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_copyback_user()\n");
#else
# error "wpabs_skb_copyback_user() function is not supported yet!"
#endif
	return 0;
}
*/

/*
** wpabs_skb_copyback() - 
**	Copy data from an mbuf chain starting "off" bytes from the beginning,
**	continuing for "len" bytes, into the indicated buffer.
*/
/*
static __inline void wpabs_skb_copydata(void* skb, int off, int len, caddr_t cp)
{
#if defined(__LINUX__)
	if (off + len > wpabs_skb_len(skb)){
		DEBUG_ABSTRACT("wpabs_skb_copydata: Internal error (off=%d, len=%d, skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
		return;
	}
	memcpy(cp, wpabs_skb_data(skb), len);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	caddr_t		data = mtod((struct mbuf*)skb, caddr_t);

	bcopy(cp, &data[off], len); 
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_copydata()\n");

	if (off + len > wpabs_skb_len(skb)){
		DEBUG_ABSTRACT("wpabs_skb_copydata: Internal error (off=%d, len=%d, skb_len=%d)!\n",
					off, len, wpabs_skb_len(skb));
		return;
	}
	memcpy(cp, wpabs_skb_data(skb), len);

#else
# error "wpabs_skb_copydata() function is not supported yet!"
#endif
}
*/

/*
** wpabs_skb_correct() - 
**		Correct skb block.
*/
/*
static __inline void wpabs_skb_correct(void* skb_dst, void* skb_src)
{
#if defined(__LINUX__)
	return;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	netskb_t*	m_dst = (netskb_t*)skb_dst;
	netskb_t*	m_src = (netskb_t*)skb_src;
	if (!m_dst || !m_src){
		return;
	}
	while(m_src != NULL){
		wpabs_skb_copyback(m_dst, wpabs_skb_len(m_dst),
				 wpabs_skb_len(m_src), wpabs_skb_data(m_src));
		m_src = m_src->m_next;
	}
	return;
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_correct()\n");
	return;
#else
# error "wpabs_skb_correct() function is not supported yet!"
#endif
}
*/


//////////////////////////////////////////////////////////////////////////////////

/* Trims skb to length len. It can change skb pointers, if "realloc" is 1.
* If realloc==0 and trimming is impossible without change of data,
* it is BUG().
*/
/*
static int ___pskb_trim(struct sk_buff *skb, unsigned int len, int realloc)
{
	unsigned int offset;

	DEBUG_ABSTRACT("___skb_trim()\n");
	
	offset = wpabs_skb_headroom(skb);

	if (offset < len) {
		skb->data_len -= skb->len - len;
		skb->len = len;
	} else {
		if (len <= (unsigned int)wpabs_skb_headroom(skb)) {
			skb->len = len;
			skb->data_len = 0;
			skb->tail = skb->data + len;
		} else {
			skb->data_len -= skb->len - len;
			skb->len = len;
		}
	}
	
	return 0;
}

static void __skb_trim(struct sk_buff *skb, unsigned int len)
{
	DEBUG_ABSTRACT("__skb_trim()\n");

	if(!skb->data_len){
		skb->len = len;
		skb->tail = skb->data+len;
	} else {
		___pskb_trim(skb, len, 0);
	}
}
*/

/*
** wpabs_skb_trim() - Trim from tail
**			
** 
*/
/*
static __inline void wpabs_skb_trim(struct sk_buff* skb, unsigned int len)
{
#if defined(__LINUX__)
	skb_trim(skb, len);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	m = (struct mbuf*)skb;
	if (len == 0){
		m->m_data 	= m->m_ext.ext_buf;
	}
	m->m_pkthdr.len	= len;
	m->m_len	= m->m_pkthdr.len;

#elif defined(__WINDOWS__)
*/
	/**
	 *      skb_trim - remove end from a buffer
	 *      @skb: buffer to alter
	 *      @len: new length
	 *
	 *      Cut the length of a buffer down by removing data from the tail. If
	 *      the buffer is already under the length specified it is not modified.
	 */
/*
	DEBUG_ABSTRACT("wpabs_skb_trim()\n");

	if (skb->len > len) {
		__skb_trim(skb, len);
    }else{
		DEBUG_ABSTRACT("wpabs_skb_trim(): 'skb->len <= len'\n");
	}
#else
# error "wpabs_skb_trim() function is not supported yet!"
#endif
}
*/

//////////////////////////////////////////////////////////////////////////////////

/*
** wpabs_skb_init() - Setup skb data ptr
**			
** 
*/
static __inline void wpabs_skb_init(void* pskb, unsigned int len)
{
#if defined(__LINUX__)
	struct sk_buff* skb = (struct sk_buff*)pskb;
	skb->data = skb->head + len;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#elif defined(__WINDOWS__)
	struct sk_buff* skb = (struct sk_buff*)pskb;

	DEBUG_ABSTRACT("wpabs_skb_init()\n");
	skb->data = skb->head + len;
#else
# error "wpabs_skb_init() function is not supported yet!"
#endif
}

/*
** wpabs_skb_tailroom() - Tail room
**			
** 
*/
static __inline int wpabs_skb_tailroom(const struct sk_buff *skb)
{
#if defined(__LINUX__)
	return skb_tailroom(skb);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	m = (struct mbuf*)skb;

	if (m->m_flags & M_EXT){
		return (MCLBYTES - m->m_len);
	}
	return (MHLEN - m->m_len);
#elif defined(__WINDOWS__)
	/**
	 *      skb_tailroom - bytes at buffer end
	 *      @skb: buffer to check
	 *
	 *      Return the number of bytes of free space at the tail of an sk_buff
	 */
	DEBUG_ABSTRACT("wpabs_skb_tailroom()\n");

	return skb->end - skb->tail;
#else
# error "wpabs_skb_tailroom() function is not supported yet!"
#endif
}

/*
** wpabs_skb_headroom() - Head room
**			
** 
**	Return the number of bytes of free space at the head of an &sk_buff.
*/
static __inline int wpabs_skb_headroom(const struct sk_buff* skb)
{
#if defined(__LINUX__)
	return skb_headroom(skb);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return 0;
#elif defined(__WINDOWS__)

	DEBUG_ABSTRACT("wpabs_skb_headroom()\n");
	return skb->data - skb->head;

#else
# error "wpabs_skb_headroom() function is not supported yet!"
#endif
}


/*
** wpabs_skb_put() - 
**
*/
static __inline unsigned char* wpabs_skb_put(struct sk_buff *skb, int len)
{
#if defined(__LINUX__)
	return skb_put((struct sk_buff*)skb, len);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	struct mbuf*	m = (struct mbuf*)skb;

	m->m_len 	= wpabs_skb_len(skb) + len;
	m->m_pkthdr.len = wpabs_skb_len(skb) + len;
	return wpabs_skb_data(skb);
#elif defined(__WINDOWS__)
	/**
	 *      skb_put - add data to a buffer
	 *      @skb: buffer to use 
	 *      @len: amount of data to add
	 *
	 *      This function extends the used data area of the buffer. If this would
	 *      exceed the total buffer size the kernel will panic. A pointer to the
	 *      first byte of the extra data is returned.
	 */
	//unsigned char* tmp = skb->data;
	unsigned char* tmp = skb->tail;

	DEBUG_ABSTRACT("wpabs_skb_put(): len: %d\n", len);

	//if(skb->data + len > skb->end) {
	if(skb->tail + len > skb->end) {
		DEBUG_EVENT(
			"wpabs_skb_put(): skb_over_panic!! Data longer than allocated 'skb' buffer.\n");
	}else{
		//skb->data += len;
		skb->tail += len;
		skb->len += len;
	}

	DEBUG_ABSTRACT("wpabs_skb_put(): skb->data: 0x%p, skb->tail: 0x%p, skb->len: %d\n",
		skb->data, skb->tail, skb->len);

	return tmp;
#else
# error "wpabs_skb_put() function is not supported yet!"
#endif
}


/*
** wpabs_skb_len() - 
**		Returns current kernel buffer length.
*/
static __inline unsigned short wpabs_skb_len(void* skb)
{
#if defined(__LINUX__)
	return ((struct sk_buff*)skb)->len;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return ((struct mbuf*)skb)->m_len;
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_len()\n");
	return (unsigned short)((struct sk_buff*)skb)->len;
#else
# error "wpabs_skb_len() function is not supported yet!"
#endif
}

/*
static char *__skb_pull(struct sk_buff *skb, unsigned int len)
{
	skb->len -= len;
	if (skb->len < skb->data_len){
		DEBUG_ABSTRACT("__skb_pull(): 'skb->len < skb->data_len'\n");
		//BUG();
	}
	return skb->data += len;
}
*/

/*
** wpabs_skb_reserve() - 
**		Adjust headroom.
*/
static __inline void wpabs_skb_reserve(struct sk_buff* skb, unsigned int len)
{
#if defined(__LINUX__)
	skb_reserve(skb, len);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)

#elif defined(__WINDOWS__)
	/**
	 *      skb_reserve - adjust headroom
	 *      @skb: buffer to alter
	 *      @len: bytes to move
	 *
	 *      Increase the headroom of an EMPTY &sk_buff by reducing the tail
	 *      room. This is only allowed for an empty buffer.
	 */
	DEBUG_ABSTRACT("wpabs_skb_reserve()\n");

	skb->data+=len;
    skb->tail+=len;
	//skb->len += len;//???

	DEBUG_ABSTRACT("wpabs_skb_reserve(): skb->data: 0x%p, skb->tail: 0x%p, skb->len: %d\n",
		skb->data, skb->tail, skb->len);

#else
# error "wpabs_skb_reserve() function is not supported yet!"
#endif
}

/*
** wpabs_skb_pull() - 
**
*/
static __inline unsigned char* wpabs_skb_pull(struct sk_buff* skb, unsigned int len)
{
#if defined(__LINUX__)
	return skb_pull((struct sk_buff*)skb, len);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	m_adj(skb, len);
#if 0
	struct mbuf*	m = (struct mbuf*)skb;
	m->m_data += len;
	m->m_pkthdr.len -= len;
	m->m_len 	= m->m_pkthdr.len;
#endif
	return wpabs_skb_data(skb);
#elif defined(__WINDOWS__)
	/**
	*      skb_pull - remove data from the start of a buffer
	*      @skb: buffer to use 
	*      @len: amount of data to remove
	*
	*      This function removes data from the start of a buffer, returning
	*      the memory to the headroom. A pointer to the new data area in the buffer
	*      is returned. Once the data has been pulled future pushes will overwrite
	*      the old data.
	*/
	DEBUG_ABSTRACT("wpabs_skb_pull()\n");
    
	if(skb->data + len > skb->end){
		DEBUG_ABSTRACT("wpabs_skb_pull(): can not 'pull' beyond 'skb->end'!!\n");
		return NULL;
	}

	if(len > skb->len){
		skb->len = 0;
	}else{
		skb->len -= len;
	}

	return skb->data += len;

#else
# error "wpabs_skb_pull() function is not supported yet!"
#endif
}

/**
 *      skb_push - add data to the start of a buffer
 *      @skb: buffer to use 
 *      @len: amount of data to add
 *
 *      This function extends the used data area of the buffer at the buffer
 *      START. If this would exceed the total buffer headroom the kernel will
 *      panic. A pointer to the first byte of the extra data is returned.
 */
static unsigned char *wpabs_skb_push(struct sk_buff *skb, unsigned int len)
{
	if(wpabs_skb_headroom(skb) < (int)len ){
		DEBUG_ABSTRACT("wpabs_skb_push(): can not 'push' more than 'headroom'!!\n");
		return NULL;
	}

	if(skb->data - len < skb->head) {
		DEBUG_ABSTRACT("wpabs_skb_push(): can not 'push' beyond 'skb->head'!!\n");
		return NULL;
    }

	skb->data -= len;
	skb->len += len;

	DEBUG_ABSTRACT("wpabs_skb_push(): skb->data: 0x%p, skb->tail: 0x%p, skb->len: %d\n",
		skb->data, skb->tail, skb->len);

    return skb->data;
}


/*
** wpabs_skb_data() - 
**		Returns pointer to data.
*/
static __inline unsigned char* wpabs_skb_data(void* skb)
{
#if defined(__LINUX__)
	return ((struct sk_buff*)skb)->data;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return mtod((struct mbuf*)skb, caddr_t);
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_data()\n");
	return ((struct sk_buff*)skb)->data;
#else
# error "wpabs_skb_data() function is not supported yet!"
#endif
}

/*
** wpabs_skb_tail() - 
**		Returns pointer to data.
*/
static __inline unsigned char* wpabs_skb_tail(void* skb)
{
#if defined(__LINUX__)
	return ((struct sk_buff*)skb)->tail;
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	return mtod((struct mbuf*)skb, caddr_t);
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_tail()\n");
	return ((struct sk_buff*)skb)->tail;
#else
# error "wpabs_skb_data() function is not supported yet!"
#endif
}

/*
** wpabs_skb_free() - 
**		Free kernel memory buffer.
*/
static __inline void wpabs_skb_free(struct sk_buff * skb)
{
#if defined(__LINUX__)
#if defined(WAN_DEBUG_MEM)
	DEBUG_SUB_MEM(((struct sk_buff*)skb)->truesize);
#endif
	dev_kfree_skb_any(skb);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	m_freem(skb);
#elif defined(__WINDOWS__)

	DEBUG_ABSTRACT("wpabs_skb_free()\n");

	if(skb == NULL){
		DEBUG_ABSTRACT("attempt to free NULL 'skb' buffer!!()\n");
		return;
	}

	if(((struct sk_buff *)skb)->head == NULL){
		DEBUG_ABSTRACT("attempt to free NULL 'skb->head' buffer!!()\n");
		return;
	}

	wpabs_free(skb->head);	//deallocate data area
	wpabs_free(skb);		//deallocate skb itself

#else
# error "wpabs_skb_free() function is not supported yet!"
#endif
}

static int wpabs_get_random_bytes(void* input, int size)
{
	DBG_NOT_IMPL(("wpabs_get_random_bytes()\n"));
	return 0;
}

static int skb_headlen(const struct sk_buff *skb)
{
	DEBUG_ABSTRACT("skb_headlen()\n");

	return skb->data - skb->head;
}


//////////////////////////////////////////////////////////////////////////////////

static __inline void wpabs_skb_set_protocol(void* pskb, unsigned int protocol)
{
#if defined(__LINUX__)
	struct sk_buff *skb = (struct sk_buff*)pskb;
	if (skb){
		skb->protocol = htons(protocol);
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	struct mbuf	*mbuf = (struct mbuf*)pskb;
	if (protocol == ETH_P_IPX){
		mbuf->m_flags |= M_PROTO5;
	}
#elif defined(__WINDOWS__)
	struct sk_buff *skb = (struct sk_buff*)pskb;

	DEBUG_ABSTRACT("wan_skb_set_protocol()\n");

	if (skb){
		skb->protocol = (unsigned char)wpabs_htons((unsigned short)protocol);
	}
#else
# warning "wan_skb_set_protocol() function is not supported yet!"
#endif
}


static __inline void wpabs_skb_set_raw(void* pskb)
{
#if defined(__LINUX__)
	struct sk_buff *skb = (struct sk_buff*)pskb;
	if (skb){
		skb->mac.raw = skb->data;
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)

#elif defined(__WINDOWS__)
	struct sk_buff *skb = (struct sk_buff*)pskb;

	DEBUG_ABSTRACT("wan_skb_set_raw()\n");

	if (skb){
//		skb->mac.raw = skb->data;
	}
#else
# warning "wan_skb_set_raw() function is not supported yet!"
#endif
}

/*
** wpabs_skb_set_dev() - 
**		Set device point.
*/
static __inline void wpabs_skb_set_dev(void* pskb, void* dev)
{
#if defined(__LINUX__)
	struct sk_buff *skb = (struct sk_buff*)pskb;
	if (skb){
		skb->dev = dev;
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	netskb_t*	m = (netskb_t*)pskb;
	if (m){
		m->m_pkthdr.rcvif = dev;
	}
#elif defined(__WINDOWS__)
	struct sk_buff *skb = (struct sk_buff*)pskb;

	DEBUG_ABSTRACT("wpabs_skb_set_dev()\n");

	if (skb){
		//skb->dev = dev;
	}
#else
# error "wan_skb_set_dev() function is not supported yet!"
#endif
}

static void wpabs_spin_lock_init(wan_spinlock_t *spinlock)
{
	WAN_SPIN_LOCK_INIT(&spinlock->slock);
}

/*
** wpabs_skb_queue_init() - 
**
*/
static __inline void wpabs_skb_queue_init(void *list)
{
#if defined(__LINUX__)
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
#elif defined(__WINDOWS__)
	wan_skb_queue_t* skb_q = (wan_skb_queue_t*)list;

	DEBUG_ABSTRACT("wpabs_skb_queue_init()\n");

	//skb_q->maximum_size = maximum_size;
	skb_q->size = 0;
	skb_q->head = NULL;
	skb_q->tail = NULL;

#else
# error "wpabs_skb_queue_init() function is not supported yet!"
#endif
}


/*
** wpabs_skb_queue_purge() - 
**
*/
static __inline void wpabs_skb_queue_purge(void *list)
{
#if defined(__LINUX__)
	struct sk_buff *skb;
	while ((skb=wpabs_skb_dequeue(list))!=NULL){
		wpabs_skb_free(skb);
	}
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	IFQ_PURGE(((wpabs_skb_queue_t*)list));
#elif defined(__WINDOWS__)
	struct sk_buff *skb;

	DEBUG_ABSTRACT("wpabs_skb_queue_purge()\n");

	while ((skb=wpabs_skb_dequeue(list)) != NULL){
		wpabs_skb_free(skb);
	}
#else
# error "wpabs_skb_queue_purge() function is not supported yet!"
#endif
}

/*
** wpabs_skb_queue_tail() - 
**
*/
static __inline void wpabs_skb_queue_tail(void* list, void* newsk)
{
#if defined(__LINUX__)
	skb_queue_tail(list, newsk);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	IF_ENQUEUE((wpabs_skb_queue_t*)list, (struct mbuf*)newsk);
#elif defined(__WINDOWS__)

	wan_skb_queue_t * skb_q = (wan_skb_queue_t*)list;
	skb_queue_element_t * element;

	DEBUG_ABSTRACT("wpabs_skb_queue_tail()\n");
/*
	if(skb_q->size >= skb_q->maximum_size){
		DEBUG_EVENT("Trace queue is full!!\n");
		return;
	}
*/
	//try allocate memory for the new element of the queue.
	element = (skb_queue_element_t *)wpabs_malloc(sizeof(skb_queue_element_t));
	if(element == NULL){
		return;
	}

	element->data = (struct sk_buff *)newsk;

	//insert element at the tail of the rx queue
	element->previous_element = NULL;

	if(skb_q->size == 0){
		//special case of a previously empty queue
		skb_q->head = element;
		skb_q->tail = element;
	}else{
		skb_q->tail->previous_element = element;
		skb_q->tail = element;
	}

	skb_q->size++;

	//DbgPrint("wpabs_skb_queue_tail(): skb_q->size: %u\n", skb_q->size);	

#else
# error "wpabs_skb_queue_tail() function is not supported yet!"
#endif
}

/*
** wpabs_skb_queue_head() - 
**
*/
static __inline void wpabs_skb_queue_head(void* list, void* newsk)
{
#if defined(__LINUX__)
	skb_queue_head(list, newsk);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	IF_PREPEND((wpabs_skb_queue_t*)list, (struct mbuf*)newsk);
#elif defined(__WINDOWS__)
	DBG_NOT_IMPL(("wpabs_skb_queue_head()"));
#endif
}

/*
** wpabs_skb_queue_len() - 
**
*/
static __inline int wpabs_skb_queue_len(void* list)
{
#if defined(__LINUX__)
	return skb_queue_len(list);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	wpabs_skb_queue_t* ifqueue = (wpabs_skb_queue_t*)list;
	return IFQ_LEN(ifqueue);
#elif defined(__WINDOWS__)
	DEBUG_ABSTRACT("wpabs_skb_queue_len()\n");
	return ((wan_skb_queue_t*)list)->size;
#else
# error "wpabs_skb_queue_len() function is not supported yet!"
#endif
}

/*
** wpabs_skb_dequeue() - 
**
*/
static __inline void *wpabs_skb_dequeue(void* list)
{
#if defined(__LINUX__)
	return skb_dequeue(list);
#elif defined(__FreeBSD__) || defined(__OpenBSD__)
	netskb_t*	newsk = NULL;
	
	IF_DEQUEUE((wpabs_skb_queue_t*)list, newsk);
	return (void*)newsk;
#elif defined(__WINDOWS__)
	//removes and returns element at the head of a queue.
	//the caller must deallocate skb!!

	wan_skb_queue_t * skb_q = (wan_skb_queue_t*)list;
	skb_queue_element_t * element;
	struct sk_buff * skb;

    DEBUG_ABSTRACT("skb_dequeue()\n");
    DEBUG_ABSTRACT("skb_dequeue(): skb_q->size: %d\n", skb_q->size);
	if(	skb_q->size == 0){
		return NULL;
	}

	//remove from the head of the rx queue
	element = skb_q->head;

	skb_q->head = (skb_queue_element_t *)element->previous_element;

	//pointer to skb with data
	skb = element->data;

	wpabs_free(element);

	//DbgPrint("1. skb_dequeue(): skb_q->size: %u\n", skb_q->size);	
	skb_q->size--;
	//DbgPrint("2. skb_dequeue(): skb_q->size: %u\n", skb_q->size);	

	if(skb_q->size == 0){
		skb_q->head = NULL;
		skb_q->tail = NULL;
	}
	//return pointer to skb with data
	return skb;

#else
# error "wpabs_skb_dequeue() function is not supported yet!"
#endif
}

///////////////////////////////////////////////////////////////////////////////

static int get_skb_queue_state(void * net_dev)
{
	netdevice_t * sdla_net_device;
	wan_skb_queue_t * skb_q;

	DEBUG_ABSTRACT("get_skb_queue_state()\n");
/*
	sdla_net_device = (netdevice_t *)net_dev;
	skb_q = &sdla_net_device->skb_q;


	//DbgPrint("skb_q->size: %u\n", skb_q->size);

	if(skb_q->size >= MAX_RX_QUEUE_SIZE){
		if(skb_q->skb_queue_full_message_printed == FALSE){	
			DbgPrint("get_skb_queue_state(): queue is full!!\n");
			//DEBUG_ISR("extension->skb_queue.size : %d\n", extension->skb_queue.size);
			//DbgBreakPoint();
			OutputLogString( "%s: Rx queue is full!\n", sdla_net_device->name);
			skb_q->skb_queue_full_message_printed = TRUE;
		}
		return EFAULT;
	}else{
		skb_q->skb_queue_full_message_printed = FALSE;
	}
*/
	return 1; //always indicate queue is full - to avoid any usage for now
	//return 0;
}

/*
static __inline void skb_unlink(void* skb)
{
	DBG_NOT_IMPL(("skb_unlink()"));
}
*/

//////////////////////////////////////////////////////////////////////////////////
//debugging function. prints content of 'skb'
//
static void wpabs_print_skb(struct sk_buff*	skb)
{
	void*			buf;
	unsigned char*	skb_data;
	int				i;

	DEBUG_ABSTRACT("print_skb()\n");

	skb_data = wpabs_skb_data(skb);

	DEBUG_ABSTRACT("DATA len: %d\n", skb->len);

	DEBUG_ABSTRACT("skb DATA:\n");

	for(i = 0; i < (int)skb->len; i++){
		if(i%15 == 0){
			DEBUG_ABSTRACT("\n");
		}
		DEBUG_ABSTRACT("%02X ", skb_data[i]);
	}

	DEBUG_ABSTRACT("\n endof skb DATA.\n");
}

#define WAN_IFQ_INIT(ifq, max_pkt)		wpabs_skb_queue_init(ifq)
#define WAN_IFQ_PURGE(ifq)				wpabs_skb_queue_purge(ifq)

#define WAN_IFQ_ENQUEUE(ifq, skb, arg, err)	\
										wpabs_skb_queue_tail((ifq), (skb))

#define WAN_IFQ_LEN(ifq)				wpabs_skb_queue_len((ifq))


/********************** WANPIPE TIMER FUNCTION **************************/

static __inline int wan_getcurrenttime(unsigned long *sec, unsigned long *usec)
{
#if defined(__FreeBSD__) || defined(__OpenBSD__)
	struct timeval 	tv;
	microtime(&tv);
	if (sec) *sec = tv.tv_sec;
	if (usec) *usec = tv.tv_usec;
	return 0;
#elif defined(__WINDOWS__)
	LARGE_INTEGER	tv;
	//NdisGetCurrentSystemTime(&tv);
	KeQuerySystemTime(&tv);
	if (sec) *sec = (unsigned long)tv.QuadPart;
	return 0;
#elif defined(__LINUX__)
	struct timeval 	tv;
	do_gettimeofday(&tv);
	if (sec) *sec = tv.tv_sec;
	if (usec) *usec = tv.tv_usec;
	return 0;
#else
# error "wan_getcurrenttime() function is not supported yet!"
#endif
}

/*
** wan_init_timer
*/
static __inline void
wan_init_timer(wan_timer_t* wan_timer, wan_timer_func_t timer_func, 
#if defined(__WINDOWS__)			   
			   wan_timer_arg_t arg)
#else
			   unsigned long arg)
#endif
{
#if defined(__LINUX__)
	init_timer(&wan_timer->timer_info);
	wan_timer->timer_info.function = timer_func;
	wan_timer->timer_info.data = arg;
#elif defined(__FreeBSD__)
	/* FIXME_ADSL_TIMER */
	callout_handle_init(&wan_timer->timer_info);
	wan_timer->timer_func = timer_func;
	wan_timer->timer_arg  = (void*)arg;
#elif defined(__OpenBSD__)
	timeout_set(&wan_timer->timer_info, timer_func, (void*)arg);
	wan_timer->timer_func = timer_func;
	wan_timer->timer_arg  = (void*)arg;
#elif defined(__WINDOWS__)
	int rc;

	VERIFY_PASSIVE_IRQL(rc);
	if(rc){
		return;
	}

	wan_timer->timer_info.function = timer_func;
	wan_timer->timer_info.context = arg;

	KeInitializeDpc(&wan_timer->timer_info.TimerDpcObject,
					wan_timer->timer_info.function,
					wan_timer->timer_info.context
					);
	KeInitializeTimer(&wan_timer->timer_info.Timer);

#else
# error "wan_init_timer() function is not supported yet!"
#endif /* linux */
}

/*
** wan_add_timer
*/
static __inline void 
wan_add_timer(wan_timer_t* wan_timer, unsigned long delay)
{
#if defined(__LINUX__)
    	wan_timer->timer_info.expires = SYSTEM_TICKS + delay;
    	add_timer(&wan_timer->timer_info);
#elif defined(__FreeBSD__)
	wan_timer->timer_info = 
			timeout(wan_timer->timer_func,
				(void*)wan_timer->timer_arg, 
				delay);
	WAN_ASSERT1(wan_timer->timer_info.callout == NULL);
#elif defined(__OpenBSD__)
	timeout_add(&wan_timer->timer_info, delay);
#elif defined(__WINDOWS__)
	LARGE_INTEGER	large_int_delay;
	int rc=1;

	//delay is in milliseconds
	DEBUG_TIMER("wan_add_timer(): delay: %u\n", delay);

	VERIFY_DISPATCH_IRQL(rc);
	if(rc){
		return;
	}

	//delay is in milliseconds
	//RtlConvertLongToLargeInteger(-10000000L * 1) - this is 1 second
	//converted to milliseconds:
	large_int_delay = RtlConvertLongToLargeInteger(-10000L * delay);

	KeSetTimer( &wan_timer->timer_info.Timer,
				large_int_delay,
				&wan_timer->timer_info.TimerDpcObject);
#else
# error "wan_add_timer() function is not supported yet!"
#endif /* linux */
}

/*
** wan_del_timer
*/
static __inline void
wan_del_timer(wan_timer_t* wan_timer)
{
#if defined(__LINUX__)
	del_timer(&wan_timer->timer_info);
#elif defined(__FreeBSD__)
	untimeout(wan_timer->timer_func,
		  (void*)wan_timer->timer_arg, 
		  wan_timer->timer_info);
	callout_handle_init(&wan_timer->timer_info);
#elif defined(__OpenBSD__)
	timeout_del(&wan_timer->timer_info);
#elif defined(__WINDOWS__)
	int rc=1;

	//DBG_WANPIPE_ABSTR(("wan_del_timer()\n"));

	VERIFY_DISPATCH_IRQL(rc);
	if(rc){
		return;
	}

	KeCancelTimer(&wan_timer->timer_info.Timer);
#else
# error "wan_del_timer() function is not supported yet!"
#endif /* linux */
}

/*
** wan_skb_csum() - 
**		Return checksum value.
*/
static __inline unsigned int wan_skb_csum(void* skb)
{
#if defined(__LINUX__)
	struct sk_buff *sk = (struct sk_buff*)skb;
	return (sk) ? sk->csum : 0;
#elif defined(__WINDOWS__)
	struct sk_buff *sk = (struct sk_buff*)skb;

	DBG_WANPIPE_ABSTR(("wan_skb_csum()\n"));

	return (sk) ? sk->csum : 0;
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	netskb_t*	m = (netskb_t*)skb;
# if defined(__OpenBSD__) && defined(OpenBSD3_7)
	return (m) ? m->m_pkthdr.csum_flags : 0;
# elif defined(__OpenBSD__)
	return (m) ? m->m_pkthdr.csum : 0;
# else
	return (m) ? m->m_pkthdr.csum_data : 0;
# endif
#else
# error "wan_skb_set_dev() function is not supported yet!"
#endif
}

#define wan_skb_pull	wpabs_skb_pull
#define wan_skb_put		wpabs_skb_put
#define wan_skb_alloc	wpabs_skb_alloc
#define wan_skb_tail	wpabs_skb_tail
#define wan_skb_init	wpabs_skb_init
#define wan_skb_len		wpabs_skb_len
#define wan_skb_data	wpabs_skb_data
#define wan_skb_queue_tail	wpabs_skb_queue_tail
#define wan_skb_dequeue		wpabs_skb_dequeue
#define wan_skb_queue_head	wpabs_skb_queue_head
#define wan_skb_free	wpabs_skb_free
#define wan_skb_queue_len	wpabs_skb_queue_len
#define wan_skb_headroom	wpabs_skb_headroom
#define wan_skb_push		wpabs_skb_push
#define wan_skb_tailroom	wpabs_skb_tailroom
#define wan_skb_queue_init wpabs_skb_queue_init
#define skb_queue_head_init wpabs_skb_queue_init

/*
** wan_skb_trim() - Trim from tail
**			
** 
*/
static __inline void wan_skb_trim(void* skb, unsigned int len)
{
#if defined(__LINUX__)
	skb_trim(skb, len);
#elif defined(__WINDOWS__)

	DBG_NOT_IMPL(("wan_skb_trim()\n"));

#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
	struct mbuf*	m = (struct mbuf*)skb;
	if (len == 0){
		m->m_data 	= m->m_ext.ext_buf;
	}
	m->m_pkthdr.len	= len;
	m->m_len	= m->m_pkthdr.len;
#else
# error "wan_skb_trim() function is not supported yet!"
#endif
}

/*
** wan_skb_set_csum() - 
**		Set checksum.
*/
static __inline void wan_skb_set_csum(void* skb, unsigned int csum)
{
#if defined(__LINUX__)
	struct sk_buff *sk = (struct sk_buff*)skb;
	if (sk){
		sk->csum = csum;
	}
#elif defined(__WINDOWS__)
	struct sk_buff *sk = (struct sk_buff*)skb;
	if (sk){
		sk->csum = csum;
	}
#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
	netskb_t*	m = (netskb_t*)skb;
	if (m){
# if defined(__OpenBSD__) && defined(OpenBSD3_7)
		m->m_pkthdr.csum_flags = csum;
# elif defined(__OpenBSD__)
		m->m_pkthdr.csum = csum;
# else
		m->m_pkthdr.csum_data = csum;
# endif
	}
#else
# error "wan_skb_set_csum() function is not supported yet!"
#endif
}

//allocate DMA buffer
static unsigned long cpu_to_le32(unsigned long addr)
{
	DBG_NOT_IMPL(("cpu_to_le32()\n"));
	return 0;
}

static void do_gettimeofday(struct timeval *tv)
{
	TIME_FIELDS			TimeFieldes;
	LARGE_INTEGER		CurrentSystemTime, CurrentLocalTime;

	KeQuerySystemTime( &CurrentSystemTime );
	ExSystemTimeToLocalTime(&CurrentSystemTime,	&CurrentLocalTime );
	RtlTimeToTimeFields( &CurrentLocalTime, &TimeFieldes );

	tv->tv_sec = TimeFieldes.Second;
	tv->tv_usec = TimeFieldes.Milliseconds;
}

static int WAN_COPY_FROM_USER(void *kernel, void *user, int length)
{
	int rc, err;

//	DEBUG_FIRMWARE_UPDATE("WAN_COPY_FROM_USER()\n");

	/*
	If running at PASSIVE_LEVEL and in the calling thread's context,
	then the address is valid and a page fault is ACCEPTABLE.
	Just access the user-mode buffer in this case.
	*/
	VERIFY_PASSIVE_IRQL(rc);
	if(rc == 0){
		memcpy(kernel, user, length);
		err = 0;
	}else{
		err = 1;
	}
	return err;
}

static int WAN_COPY_TO_USER(void *user, void *kernel, int length)
{
	int rc, err;

//	DEBUG_FIRMWARE_UPDATE("WAN_COPY_TO_USER()\n");

	/*
	If running at PASSIVE_LEVEL and in the calling thread's context,
	then the address is valid and a page fault is ACCEPTABLE.
	Just access the user-mode buffer in this case.
	*/
	VERIFY_PASSIVE_IRQL(rc);
	if(rc == 0){
		memcpy(user, kernel, length);
		err = 0;
	}else{
		err = 1;
	}
	return err;
}

// convert timeout in Milliseconds to relative timeout in 100ns units
//   suitable as parameter 5 to KeWaitForSingleObject(..., TimeOut)
#define PPT_SET_RELATIVE_TIMEOUT_IN_MILLISECONDS(VARIABLE, VALUE) (VARIABLE).QuadPart = -( (LONGLONG) (VALUE)*10*1000 )

#define wan_skb_queue_purge	wpabs_skb_queue_purge
#define wan_spin_lock_init	wpabs_spin_lock_init

#define wan_malloc	wpabs_malloc
#define wan_vmalloc wpabs_malloc

#define wan_free	wpabs_free
#define wan_vfree	wpabs_free

#define strlcpy	strncpy

#endif//#ifndef _WANPIPE_ABSTR_H
