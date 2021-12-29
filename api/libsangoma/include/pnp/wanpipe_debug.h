/*
 ************************************************************************
 * wanpipe_debug.h	WANPIPE(tm) 	Global definition for Sangoma 	*
 *					Debugging messages		*
 *									*
 * Author:		Alex Feldman <al.feldman@sangoma.com>		*
 *======================================================================*
 *	May 10 2002		Alex Feldman	Initial version		*
 *									*
 ************************************************************************
 */

#ifndef __WANPIPE_DEBUG_H
# define __WANPIPE_DEBUG_H

//#define DEBUG_LIP			DbgPrint
#define DEBUG_LIP

//#define DEBUG_ABSTRACT	DbgPrint
#define DEBUG_ABSTRACT

//#define DBG_TE1_INTERRUPT	DbgPrint
#define DBG_TE1_INTERRUPT

//#define DEBUG_NEW_TX	DbgPrint
#define DEBUG_NEW_TX

//#define DEBUG_CHDLC	DbgPrint
#define DEBUG_CHDLC

//#define DEBUG_IF_TX	DbgPrint
#define DEBUG_IF_TX

//#define DEBUG_IF_RX	DbgPrint
#define DEBUG_IF_RX

//#define DEBUG_COMMON	DbgPrint
#define DEBUG_COMMON

//#define DEBUG_REQUEST	DbgPrint
#define DEBUG_REQUEST

//#define DBG_FAST_TX	DbgPrint
#define DBG_FAST_TX

//#define DBG_FAST_TX_V1	DbgPrint
#define DBG_FAST_TX_V1

//#define DBG_BSTRM			DbgPrint
#define DBG_BSTRM

//#define DEBUG_AFT			DbgPrint
#define DEBUG_AFT

//#define DEBUG_IDLE_TX		DbgPrint
#define DEBUG_IDLE_TX

//#define DEBUG_FIRMWARE_UPDATE	DbgPrint
#define DEBUG_FIRMWARE_UPDATE

//#define DBG_8TE1		DbgPrint
#define DBG_8TE1

//#define DBG_8TE1_START	DbgPrint
#define DBG_8TE1_START

#define WAN_DEBUG_EVENT

//#define WAN_DEBUG_KERNEL
//#define WAN_DEBUG_MOD

//#define WAN_DEBUG_CFG

//#define WAN_DEBUG_INIT_VAR

//#define WAN_DEBUG_IOCTL
//#define WAN_DEBUG_CMD

//#define WAN_DEBUG_ISR
//#define WAN_DEBUG_RX

//#define WAN_DEBUG_RX_ERROR
//#define WAN_DEBUG_TX
//#define WAN_DEBUG_TX_ERROR
//#define WAN_DEBUG_TIMER

//#define WAN_DEBUG_UDP

//#define WAN_DEBUG_56K
//#define WAN_DEBUG_PROCFS
//#define WAN_DEBUG_VOIP

//#define WAN_DEBUG_TEST

//#define WAN_DEBUG_DBG

//#define WAN_DEBUG_DMA

//#define WAN_DEBUG_SNMP
//#define WAN_DEBUG_MEM

#define WAN_DEBUG_TE1//must be always enabled!!!

//#define WAN_DEBUG_TDM_VOICE
//#define WAN_DEBUG_RM
//#define WAN_DEBUG_HWEC
//#define WAN_DEBUG_TDMAPI

#if defined (__WINDOWS__)


#define A104D_CODE	//this is for searching for A104D code
#define FIRM_UPDATE	//this is for searching for AFT Firmware Update code

//these are defined in sources file
#if defined( VIRTUAL_IF_DRV )
	#define DRIVER_NAME "SDLADRV"
#elif defined( BUSENUM_DRV )
	#define DRIVER_NAME "SangBus"
#elif defined( NDIS_MINIPORT_DRIVER )
	#define DRIVER_NAME "WANPIPE"
#elif defined( SPROTOCOL )
	#define DRIVER_NAME "SPROTOCOL"
#endif

void OutputLogString(PUCHAR pvFormat, ...);

///////////////////////////////////////////////////////////////////////
//errors display - only when log is NOT working for some reasons.
//visible only in Debugger. It is the last resort.
#define ERR_DBG_OUT(_x_)									\
{	DbgPrint("%s: Error in File: %s, Line: %d. Text:\n",	\
		DRIVER_NAME, __FILE__, __LINE__);                   \
	DbgPrint _x_;                                           \
}
///////////////////////////////////////////////////////////////////////
/*
void new_output_log_string(void* card, PVOID pvFormat, ...);
#define DBG_OUT(_x_)					\
{										\
	void* card=NULL;					\
	new_output_log_string(card, _x_);	\
}
*/
///////////////////////////////////////////////////////////////////////
#if defined(SANG_DBG)
 #define AFT_FUNC_DEBUG()									\
	{														\
		DbgPrint("%s:%s(): File: %s, Line: %d.\n",			\
			DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__);	\
	}
#else
 #define AFT_FUNC_DEBUG()
#endif

#define FUNC_DEBUG		AFT_FUNC_DEBUG

///////////////////////////////////////////////////////////////////////
//use for displaying "not implemented" message
#define DBG_NOT_IMPL(_x_)									\
{	DEBUG_TEST("%s: Not implemented! File: %s, Line: %d.\n",	\
		DRIVER_NAME, __FILE__, __LINE__);					\
	DEBUG_TEST("Text: ");	DEBUG_TEST _x_;						\
}
///////////////////////////////////////////////////////////////////////

# define DEBUG_NONE		if (0) DbgPrint
# define PRINT		OutputLogString
//# define PRINT			DbgPrint
# define DEBUG_PRINT	DbgPrint
# define _DEBUG_PRINT	DbgPrint

# define DEBUG_KERNEL	DEBUG_NONE
# define DEBUG_EVENT	DEBUG_NONE
# define DEBUG_MOD		DEBUG_NONE
# define DEBUG_CFG		DEBUG_NONE
# define DEBUG_INIT		DEBUG_NONE
# define DEBUG_IOCTL	DEBUG_NONE
# define DEBUG_CMD		DEBUG_NONE
# define DEBUG_ISR		DEBUG_NONE
# define DEBUG_RX		DEBUG_NONE
# define DEBUG_RX_ERR	DEBUG_NONE
# define DEBUG_TX		DEBUG_NONE
# define DEBUG_TX_ERR	DEBUG_NONE
# define DEBUG_TIMER	DEBUG_NONE
# define DEBUG_UDP		DEBUG_NONE
# define DEBUG_TE1		DEBUG_NONE
# define DEBUG_56K		DEBUG_NONE
# define DEBUG_PROCFS	DEBUG_NONE
# define DEBUG_VOIP		DEBUG_NONE
# define DEBUG_TEST		DEBUG_NONE
# define DEBUG_DBG		DEBUG_NONE
# define DEBUG_DMA		DEBUG_NONE
# define DEBUG_SNMP		DEBUG_NONE
# define DEBUG_HWEC		DEBUG_NONE
# define DEBUG_TDMV		DEBUG_NONE
# define DEBUG_TDMAPI	DEBUG_NONE
# define DEBUG_RM		DEBUG_NONE
# define TDM_FUNC_DBG
# define DEBUG_SUB_MEM
# define WAN_DEBUG_FUNC_START
# define WAN_DEBUG_FUNC_END
# define WAN_DEBUG_FUNC_LINE


# ifdef WAN_DEBUG_KERNEL
#  undef  DEBUG_KERNEL
#  define DEBUG_KERNEL	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_EVENT
#  undef  DEBUG_EVENT
#  define DEBUG_EVENT	PRINT
//#  define DEBUG_EVENT	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_MOD
#  undef  DEBUG_MOD
#  define DEBUG_MOD	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_CFG
#  undef  DEBUG_CFG
#  define DEBUG_CFG	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_INIT_VAR
#  undef  DEBUG_INIT
#  define DEBUG_INIT	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_IOCTL
#  undef  DEBUG_IOCTL
#  define DEBUG_IOCTL	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_CMD
#  undef  DEBUG_CMD
#  define DEBUG_CMD	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_ISR
#  undef  DEBUG_ISR
#  define DEBUG_ISR	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_RX
#  undef  DEBUG_RX
#  define DEBUG_RX	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_RX_ERROR
#  undef  DEBUG_RX_ERR
#  define DEBUG_RX_ERR	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TX
#  undef  DEBUG_TX
#  define DEBUG_TX	DEBUG_PRINT
#  undef  _DEBUG_TX
#  define _DEBUG_TX	_DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TX_ERROR
#  undef  DEBUG_TX_ERR
#  define DEBUG_TX_ERR	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TIMER
#  undef  DEBUG_TIMER
#  define DEBUG_TIMER	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_UDP
#  undef  DEBUG_UDP
#  define DEBUG_UDP	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TE1
#  undef  DEBUG_TE1
//#  define DEBUG_TE1	DEBUG_PRINT
#  define DEBUG_TE1	PRINT
# endif 
# ifdef WAN_DEBUG_56K
#  undef  DEBUG_56K
#  define DEBUG_56K	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_VOIP
#  undef  DEBUG_VOIP
#  define DEBUG_VOIP	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TEST
#  undef  DEBUG_TEST
#  define DEBUG_TEST	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_DBG
#  undef  DEBUG_DBG
#  define DEBUG_DBG	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_DMA
#  undef  DEBUG_DMA
#  define DEBUG_DMA	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_SNMP
#  undef  DEBUG_SNMP
#  define DEBUG_SNMP	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_RM
#  undef  DEBUG_RM
#  define DEBUG_RM	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TDM_VOICE
#  undef  DEBUG_TDMV
#  define DEBUG_TDMV	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_HWEC
#  undef  DEBUG_HWEC
#  define DEBUG_HWEC	DEBUG_PRINT
# endif
# ifdef WAN_DEBUG_TDMAPI
#  undef  DEBUG_TDMAPI
#  define DEBUG_TDMAPI	DEBUG_PRINT
#  undef  TDM_FUNC_DBG
#  define TDM_FUNC_DBG	DEBUG_TDMAPI("TDM API: %s():%d\n", __FUNCTION__, __LINE__);
# endif 


#else	/* !__WINDOWS__*/

# define DEBUG_KERNEL(format,msg...)
# define DEBUG_EVENT(format,msg...)
# define DEBUG_MOD(format,msg...)
# define DEBUG_CFG(format,msg...)
# define DEBUG_INIT(format,msg...)
# define DEBUG_IOCTL(format,msg...)
# define DEBUG_CMD(format,msg...)
# define DEBUG_ISR(format,msg...)
# define DEBUG_RX(format,msg...)
# define DEBUG_RX_ERR(format,msg...)
# define DEBUG_TX(format,msg...)	
# define _DEBUG_TX(format,msg...)
# define DEBUG_TX_ERR(format,msg...)
# define DEBUG_TIMER(format,msg...)	
# define DEBUG_UDP(format,msg...)
# define DEBUG_TE1(format,msg...)
# define DEBUG_56K(format,msg...)
# define DEBUG_PROCFS(format,msg...)
# define DEBUG_VOIP(format,msg...)
# define DEBUG_TEST(format,msg...)
# define DEBUG_DBG(format,msg...)
# define DEBUG_DMA(format,msg...)
# define DEBUG_SNMP(format,msg...)
# define DEBUG_ADD_MEM(a)
# define DEBUG_SUB_MEM(a)

# if (defined __FreeBSD__) || (defined __OpenBSD__)

#  define DEBUG_PRINT(format,msg...)	log(LOG_INFO, format, ##msg)
#  define _DEBUG_PRINT(format,msg...)   log(LOG_INFO, format, ##msg)

# else	/* !__FreeBSD__ && !__OpenBSD__ */

#  define DEBUG_PRINT(format,msg...)	printk(KERN_INFO format, ##msg)
#  define _DEBUG_PRINT(format,msg...)   printk(format,##msg)

# endif	/* __FreeBSD__ || __OpenBSD__ */

# ifdef WAN_DEBUG_KERNEL
#  undef  DEBUG_KERNEL
#  define DEBUG_KERNEL(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_EVENT
#  undef  DEBUG_EVENT
#  define DEBUG_EVENT(format,msg...)		DEBUG_PRINT(format,##msg)
#  undef  _DEBUG_EVENT
#  define _DEBUG_EVENT(format,msg...)		_DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_MOD
#  undef  DEBUG_MOD
#  define DEBUG_MOD(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_CFG
#  undef  DEBUG_CFG
#  define DEBUG_CFG(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_INIT_VAR
#  undef  DEBUG_INIT
#  define DEBUG_INIT(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_IOCTL
#  undef  DEBUG_IOCTL
#  define DEBUG_IOCTL(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_CMD
#  undef  DEBUG_CMD
#  define DEBUG_CMD(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_ISR
#  undef  DEBUG_ISR
#  define DEBUG_ISR(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_RX
#  undef  DEBUG_RX
#  define DEBUG_RX(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_RX_ERROR
#  undef  DEBUG_RX_ERR
#  define DEBUG_RX_ERR(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TX
#  undef  DEBUG_TX
#  define DEBUG_TX(format,msg...)		DEBUG_PRINT(format,##msg)
#  undef  _DEBUG_TX
#  define _DEBUG_TX(format,msg...)		_DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TX_ERROR
#  undef  DEBUG_TX_ERR
#  define DEBUG_TX_ERR(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TIMER
#  undef  DEBUG_TIMER
#  define DEBUG_TIMER(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_UDP
#  undef  DEBUG_UDP
#  define DEBUG_UDP(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TE1
#  undef  DEBUG_TE1
#  define DEBUG_TE1(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_56K
#  undef  DEBUG_56K
#  define DEBUG_56K(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_VOIP
#  undef  DEBUG_VOIP
#  define DEBUG_VOIP(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TEST
#  undef  DEBUG_TEST
#  define DEBUG_TEST(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_DBG
#  undef  DEBUG_DBG
#  define DEBUG_DBG(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_MEM
#  undef  DEBUG_ADD_MEM
#  define DEBUG_ADD_MEM(a)  /*DEBUG_EVENT("%s:%d MEM ADDING %d\n",__FUNCTION__,__LINE__,a);*/(atomic_add(a,&wan_debug_mem))
#  undef  DEBUG_SUB_MEM
#  define DEBUG_SUB_MEM(a)  /*DEBUG_EVENT("%s:%d MEM SUBSTR %d\n",__FUNCTION__,__LINE__,a);*/(atomic_sub(a,&wan_debug_mem))
#endif
# ifdef WAN_DEBUG_DMA
#  undef  DEBUG_DMA
#  define DEBUG_DMA(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_SNMP
#  undef  DEBUG_SNMP
#  define DEBUG_SNMP(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif


#endif	/* __WINDOWS__ */


#define WAN_DEBUG_FLINE	DEBUG_EVENT("[%s]: %s:%d\n",			\
				__FILE__,__FUNCTION__,__LINE__);

#if defined(WAN_DEBUG_FUNC)
# undef WAN_DEBUG_FUNC_START
# define WAN_DEBUG_FUNC_START	DEBUG_EVENT("[%s]: %s:%d: Start (%ld)\n",	\
				__FILE__,__FUNCTION__,__LINE__, SYSTEM_TICKS);
# undef WAN_DEBUG_FUNC_END
# define WAN_DEBUG_FUNC_END	DEBUG_EVENT("[%s]: %s:%d: End (%ld)\n",		\
				__FILE__,__FUNCTION__,__LINE__,SYSTEM_TICKS);
# undef WAN_DEBUG_FUNC_LINE
# define WAN_DEBUG_FUNC_LINE	DEBUG_EVENT("[%s]: %s:%d: (%ld)\n",		\
				__FILE__,__FUNCTION__,__LINE__,SYSTEM_TICKS);
#endif

#define WAN_ASSERT(val) if (val){								\
			DEBUG_EVENT("**************** ASSERT FAILED ****************\n");	\
			DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);		\
			return -EINVAL;								\
			}
#define WAN_ASSERT_EINVAL(val) WAN_ASSERT(val)

#define WAN_ASSERT1(val) if (val){								\
			DEBUG_EVENT("**************** ASSERT FAILED ****************\n");	\
			DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);		\
			return;									\
			}
#define WAN_ASSERT_VOID(val) WAN_ASSERT1(val)

#define WAN_ASSERT2(val, ret) if (val){								\
			DEBUG_EVENT("**************** ASSERT FAILED ****************\n");	\
			DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);		\
			return ret;								\
			}
#define WAN_ASSERT_RC(val,ret) WAN_ASSERT2(val, ret)

#define WAN_MEM_ASSERT(str) {if (str){ \
                          	DEBUG_EVENT("%s: Error: No memory in %s:%i\n", \
						str,__FILE__,__LINE__); \
			  }else{ \
				DEBUG_EVENT("wanpipe: Error: No memory in %s:%i\n", \
						__FILE__,__LINE__); \
			  }\
			  }

#define WAN_OPP_FLAG_ASSERT(val,cmd) if (val){\
			DEBUG_EVENT("%s:%d - Critical error: Opp Flag Set Cmd=0x%x!\n",__FILE__,__LINE__,cmd);\
			}



#define WAN_MEM_INIT(id)	unsigned long mem_in_used_##id = 0x0l
#define WAN_MEM_INC(id,size)	mem_in_used_##id += size
#define WAN_MEM_DEC(id,size)	mem_in_used_##id -= size

/* WANPIPE debugging states */
#define WAN_DEBUGGING_NONE		0x00
#define WAN_DEBUGGING_AGAIN		0x01
#define WAN_DEBUGGING_START		0x02
#define WAN_DEBUGGING_CONT		0x03
#define WAN_DEBUGGING_PROTOCOL		0x04
#define WAN_DEBUGGING_END		0x05

/* WANPIPE debugging delay */
#define WAN_DEBUGGING_DELAY		60

/* WANPIPE debugging messages */
#define WAN_DEBUG_NONE_MSG		0x00
#define WAN_DEBUG_ALARM_MSG		0x01
#define WAN_DEBUG_TE1_MSG		0x02
#define WAN_DEBUG_LINERROR_MSG		0x03
#define WAN_DEBUG_CLK_MSG		0x04
#define WAN_DEBUG_TX_MSG		0x05
#define WAN_DEBUG_FR_CPE_MSG		0x06
#define WAN_DEBUG_FR_NODE_MSG		0x07
#define WAN_DEBUG_PPP_LCP_MSG		0x08
#define WAN_DEBUG_PPP_NAK_MSG		0x09
#define WAN_DEBUG_PPP_NEG_MSG		0x0A
#define WAN_DEBUG_CHDLC_KPLV_MSG	0x0B	
#define WAN_DEBUG_CHDLC_UNKNWN_MSG	0x0C	


# define WAN_DEBUG_INIT(card){					\
	wan_tasklet_t* debug_task = &card->debug_task;		\
	WAN_TASKLET_INIT(debug_task, 0, &wanpipe_debugging, card);\
	clear_bit(0, (unsigned long*)&card->debug_running);			\
	wanpipe_debug_timer_init(card);				\
	}
#define WAN_DEBUG_END(card){					\
	wan_tasklet_t* debug_task = &card->debug_task;		\
	wan_del_timer(&card->debug_timer);			\
	WAN_TASKLET_KILL(debug_task);				\
	}
#define WAN_DEBUG_STOP(card)	clear_bit(0, &card->debug_running)

#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__WINDOWS__)
# define WAN_DEBUG_START(card)					\
		if (!test_bit(0, &card->debug_running)){	\
			set_bit(0, &card->debug_running);	\
			wan_add_timer(&card->debug_timer, 5*HZ);	\
		}

#elif defined(__LINUX__)

# define WAN_DEBUG_START(card)					\
		if (!test_and_set_bit(0, &card->debug_running)){\
			wan_add_timer(&card->debug_timer, 5*HZ);	\
		}

# define WAN_TIMEOUT(sec)  { unsigned long timeout; \
	                     timeout=jiffies; \
			     while ((jiffies-timeout)<sec*HZ){ \
				     schedule(); \
			     }\
			   }


#define WP_READ_LOCK(lock,flag)   {  DEBUG_TEST("%s:%i: RLock %u\n",__FILE__,__LINE__,(u32)lock);\
	                             read_lock(lock); flags=0; }
				     
#define WP_READ_UNLOCK(lock,flag) {  DEBUG_TEST("%s:%i: RULock %u\n",__FILE__,__LINE__,(u32)lock);\
					read_unlock(lock); flags=0; }

#define WP_WRITE_LOCK(lock,flag) {  DEBUG_TEST("%s:%i: WLock %u\n",__FILE__,__LINE__,(u32)lock); \
	                            write_lock_irqsave(lock,flag); }

#define WP_WRITE_UNLOCK(lock,flag) { DEBUG_TEST("%s:%i: WULock %u\n",__FILE__,__LINE__,(u32)lock); \
	                             write_unlock_irqrestore(lock,flag); }

#elif defined(__WINDOWS__)
//not accessible for Windows

#else
# error "Undefined WAN_DEBUG_START macro!"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////
#if defined(__WINDOWS__)
#define WP_READ_LOCK(lock,flag)   { DEBUG_TEST("%s:%i: RLock %u\n",__FILE__,__LINE__,(u32)lock);}
									//read_lock(lock); flags=0; }
				     
#define WP_READ_UNLOCK(lock,flag) { DEBUG_TEST("%s:%i: RULock %u\n",__FILE__,__LINE__,(u32)lock);}
									//read_unlock(lock); flags=0; }

#define WP_WRITE_LOCK(lock,flag) {	DEBUG_TEST("%s:%i: WLock %u\n",__FILE__,__LINE__,(u32)lock);}
									//write_lock_irqsave(lock,flag); }

#define WP_WRITE_UNLOCK(lock,flag) {DEBUG_TEST("%s:%i: WULock %u\n",__FILE__,__LINE__,(u32)lock);}
									//write_unlock_irqrestore(lock,flag); }

#endif
//////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(__LINUX__) && defined(WP_FUNC_DEBUG)

#define WP_USEC_DEFINE()            unsigned long wptimeout; struct timeval  wptv1,wptv2;
#define WP_START_TIMING()           wptimeout=jiffies; do_gettimeofday(&wptv1);
#define WP_STOP_TIMING_TEST(label,usec) { do_gettimeofday(&wptv2);\
                                    wptimeout=jiffies-wptimeout; \
		                    if (wptimeout >= 2){ \
                        	 	DEBUG_EVENT("%s:%u %s Jiffies=%lu\n",  \
                                        	__FUNCTION__,__LINE__,label,wptimeout);  \
		                    }\
				     \
                		    wptimeout=wptv2.tv_usec - wptv1.tv_usec; \
                                    if (wptimeout >= usec){ \
					DEBUG_EVENT("%s:%u %s:%s Usec=%lu\n",  \
                                                __FUNCTION__,__LINE__,card->devname,label,wptimeout);  \
                		    }\
                                  }

#else

#define WP_USEC_DEFINE()              
#define WP_START_TIMING()             
#define WP_STOP_TIMING_TEST(label,usec) 

#endif


typedef struct wanpipe_debug_hdr {
	unsigned long	magic;
	unsigned long	total_len;
} wanpipe_kernel_msg_info_t;

#define WAN_DEBUG_SET_TRIGGER	0x01
#define WAN_DEBUG_CLEAR_TRIGGER	0x02

#define WAN_DEBUG_READING	0x00
#define WAN_DEBUG_FULL		0x01
#define WAN_DEBUG_TRIGGER	0x02

extern void wan_debug_trigger(int);
extern void wan_debug_write(char*);
extern int wan_debug_read(void*, void*);

/* NC Added to debug function calls */
extern void wp_debug_func_init(void);
extern void wp_debug_func_add(unsigned char *func);
extern void wp_debug_func_print(void);


#endif /* __WANPIPE_DEBUG_H */
