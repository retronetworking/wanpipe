/*
 ************************************************************************
 * wanpipe_debug.h	WANPIPE(tm) 	Global definition for Sangoma 		*
 *					Debugging messages									*
 *																		*
 * Author:		Alex Feldman <al.feldman@sangoma.com>					*
 *======================================================================*
 *	May 10 2002		Alex Feldman	Initial version						*
 *																		*
 ************************************************************************
 */

#ifndef __WANPIPE_DEBUG_H
# define __WANPIPE_DEBUG_H

#if defined(WAN_KERNEL)

#define WAN_DEBUG_EVENT
#undef WAN_DEBUG_KERNEL
#undef WAN_DEBUG_MOD
#undef WAN_DEBUG_CFG
#undef WAN_DEBUG_REG
#undef WAN_DEBUG_INIT_VAR
#undef WAN_DEBUG_IOCTL
#undef WAN_DEBUG_CMD
#undef WAN_DEBUG_ISR
#undef WAN_DEBUG_RX
#undef WAN_DEBUG_RX_ERROR
#undef WAN_DEBUG_TX
#undef WAN_DEBUG_TX_ERROR
#undef WAN_DEBUG_TIMER
#undef WAN_DEBUG_UDP
#undef WAN_DEBUG_TE1
#undef WAN_DEBUG_56K
#undef WAN_DEBUG_A600
#undef WAN_DEBUG_PROCFS
#undef WAN_DEBUG_TDM_VOICE
#undef WAN_DEBUG_TEST
#undef WAN_DEBUG_DBG
#undef WAN_DEBUG_DMA
#undef WAN_DEBUG_SNMP
#undef WAN_DEBUG_TE3
#undef WAN_DEBUG_RM
#undef WAN_DEBUG_HWEC
#undef WAN_DEBUG_TDMAPI
#undef WAN_DEBUG_FE
#undef WAN_DEBUG_NG
#undef WAN_DEBUG_MEM
#undef WAN_DEBUG_BRI
#undef WAN_DEBUG_BRI_INIT
#undef WAN_DEBUG_USB
#undef WAN_DEBUG_FUNC

#if defined (__WINDOWS__)

void OutputLogString(PUCHAR pvFormat, ...);

# define DEBUG_NONE
#if 1
# define PRINT	OutputLogString
#else
# define PRINT	if(1)DbgPrint
#endif


# define DEBUG_PRINT	DbgPrint
# define _DEBUG_PRINT	DbgPrint

#define _DEBUG_EVENT 	DbgPrint

# define DEBUG_KERNEL	DEBUG_NONE
# define DEBUG_EVENT	DEBUG_NONE
# define DEBUG_MOD	DEBUG_NONE
# define DEBUG_CFG	DEBUG_NONE
# define DEBUG_REG	DEBUG_NONE
# define DEBUG_INIT	DEBUG_NONE
# define DEBUG_IOCTL	DEBUG_NONE
# define DEBUG_CMD	DEBUG_NONE
# define DEBUG_ISR	DEBUG_NONE
# define DEBUG_RX	DEBUG_NONE
# define DEBUG_RX_ERR	DEBUG_NONE
# define DEBUG_TX	DEBUG_NONE
# define _DEBUG_TX	DEBUG_NONE
# define DEBUG_TX_ERR	DEBUG_NONE
# define DEBUG_TIMER	DEBUG_NONE
# define DEBUG_UDP	DEBUG_NONE
# define DEBUG_TE1	DEBUG_NONE
# define DEBUG_TE3	DEBUG_NONE
# define DEBUG_56K	DEBUG_NONE
# define DEBUG_A600	DEBUG_NONE
# define DEBUG_BRI	DEBUG_NONE
# define DEBUG_PROCFS	DEBUG_NONE
# define DEBUG_TDMV	DEBUG_NONE
# define DEBUG_TEST	DEBUG_NONE
# define DEBUG_DBG	DEBUG_NONE
# define DEBUG_DMA	DEBUG_NONE
# define DEBUG_SNMP	DEBUG_NONE
# define DEBUG_RM	DEBUG_NONE
# define DEBUG_HWEC	DEBUG_NONE
# define DEBUG_TDMAPI	DEBUG_NONE
# define DEBUG_FE	DEBUG_NONE
# define DEBUG_NG	DEBUG_NONE
# define WAN_DEBUG_FUNC_START	DEBUG_NONE
# define WAN_DEBUG_FUNC_END	DEBUG_NONE
# define WAN_DEBUG_FUNC_LINE	DEBUG_NONE
# define DEBUG_BRI	DEBUG_NONE
# define DEBUG_BRI_INIT DEBUG_NONE

# ifdef WAN_DEBUG_KERNEL
#  undef  DEBUG_KERNEL
#  define DEBUG_KERNEL	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_EVENT
#  undef  DEBUG_EVENT
#  define DEBUG_EVENT	PRINT
# endif 
# ifdef WAN_DEBUG_MOD
#  undef  DEBUG_MOD
#  define DEBUG_MOD	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_CFG
#  undef  DEBUG_CFG
#  define DEBUG_CFG	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_REG
#  undef  DEBUG_REG
#  define DEBUG_REG	DEBUG_PRINT
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
#  define DEBUG_TE1	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TE3
#  undef  DEBUG_TE3
#  define DEBUG_TE3	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_56K
#  undef  DEBUG_56K
#  define DEBUG_56K	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_A600
#  undef  DEBUG_A600
#  define DEBUG_A600	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_BRI
#  undef  DEBUG_BRI
#  define DEBUG_BRI	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TDM_VOICE
#  undef  DEBUG_TDMV
#  define DEBUG_TDMV	DEBUG_PRINT
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
# ifdef WAN_DEBUG_FE
#  undef  DEBUG_FE
#  define DEBUG_FE	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_HWEC
#  undef  DEBUG_HWEC
#  define DEBUG_HWEC	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_TDMAPI
#  undef  DEBUG_TDMAPI
#  define DEBUG_TDMAPI	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_NG
#  undef  DEBUG_NG
#  define DEBUG_NG	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_BRI
#  undef  DEBUG_BRI
#  define DEBUG_BRI	DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_BRI_INIT
#  undef  DEBUG_BRI_INIT
#  define DEBUG_BRI_INIT DEBUG_PRINT
# endif 
# ifdef WAN_DEBUG_USB
#  undef  DEBUG_USB
#  define DEBUG_USB	DEBUG_PRINT
# endif 

//uncomment -DSANG_DBG in C_DEFINES in Sources to get debug output
#if defined(SANG_DBG)
 //print driver name and a line
 #define DbgOut(_f_, _x_) if(_f_) { DbgPrint("%s: ", Globals.DriverName); DbgPrint _x_;}
 //print a line. used when driver name is not needed
 #define DbgOutL(_f_, _x_) if(_f_) { DbgPrint _x_;}
 #define BUS_DEFAULT_DEBUG_OUTPUT_LEVEL 0x000FFFFF
#else //#if SANG_DBG
 #define BUS_DEFAULT_DEBUG_OUTPUT_LEVEL 0x0
 #define DbgOut(_f_, _x_)
 #define DbgOutL(_f_, _x_)
#endif//#if SANG_DBG

extern ULONG BusEnumDebugLevel;

# define DEBUG_ADD_MEM
# define DEBUG_SUB_MEM

# define splimp() 0
# define splx(l)

#define	ERR_DBG_OUT			if(0)DbgPrint
#define DBG_NOT_IMPL		if(0)DbgPrint
#define FUNC_NOT_IMPL()		if(0)DbgPrint("%s()-Not Implemented(File:%s, Line:%i)\n", __FUNCTION__, __FILE__, __LINE__)
#define DBG_DSL_NOT_IMPLD	FUNC_NOT_IMPL()

/* debugging of SngBus.sys */
#define DEBUG_SNGBUS	if(0)DbgPrint

#define DBG_SINGLE_IRQLOCK if(0)DbgPrint

/* debugging of wanpipe_kernel.h */
#define DBG_KRN			if(0)DbgPrint
#define DBG_COPY_FROM_TO_USER if(0)DbgPrint
#define DBG_SHUTDOWN	if(0)DbgPrint

#define DBG_FAST_TX		if(0)DbgPrint

#define DEBUG_SHARED_EVENT if(0)DbgPrint
#define DBG_ADSL_TX		if(0)DbgPrint
#define DBG_IRQLOCK		if(0)DbgPrint
#define DBG_ADSL_FAST_TX if(0)DbgPrint

#define DBG_SET_CFG		if(0)DbgPrint
#define DBG_ADSL_INIT	if(0)DbgPrint

#define DBG_A200_INIT	if(0)DbgPrint

#define DBG_LIP_OOB		if(0)DbgPrint
#define DEBUG_XLNX_AFT	if(0)DbgPrint
#define DEBUG_AFT		if(0)DbgPrint

#define DBG_BSTRM		if(0)DbgPrint
#define DEBUG_FIRMWARE_UPDATE if(0)DbgPrint
#define DBG_ADSL_RX		if(0)DbgPrint

#define	DBG_FE_LOCK		if(0)DbgPrint
#define DBG_DRVSTOP		if(0)DbgPrint
#define DBG_GET_REGISTRY if(0)DbgPrint
#define	DBG_FE_LOCK		if(0)DbgPrint

#define DEBUG_RX_FIFO	if(0)DbgPrint

/* sprotocol.sys */
#define DEBUG_LIP		if(0)DbgPrint
#define DBG_LIP_SKB		if(0)DbgPrint
/* wanpipe.sys */
#define DEBUG_REQUEST	if(0)DbgPrint
#define DEBUG_IF_TX		if(0)DbgPrint
#define DEBUG_COMMON	if(0)DbgPrint
#define DEBUG_IF_RX		if(0)DbgPrint
#define DEBUG_NET_IF	if(0)DbgPrint

#define DBG_RBS_EVENT	if(0)DbgPrint

#define DBG_ADSL_MACADDR	if(0)DbgPrint
#define DBG_ADSL_SYNCH		if(0)DbgPrint

#define DBG_MEM		if(0)DbgPrint

#define DBG_PNP		if(0)DbgPrint(DRIVER_NAME);if(0)DbgPrint

#define DBG_SYMB_LINK if(0)DbgPrint

#define DBG_BUFLEN	 if(0)DbgPrint

#define ADSL_TEST 0

/* These are defined in "sources" file of each driver */
#if defined( VIRTUAL_IF_DRV )
	#define DRIVER_NAME "SDLADRV"
#elif defined( BUSENUM_DRV )
	#define DRIVER_NAME "SngBus"
#elif defined( NDIS_MINIPORT_DRIVER )
	#define DRIVER_NAME "WANPIPE"
#elif defined( SPROTOCOL )
	#define DRIVER_NAME "SPROTOCOL"
#endif

#define DBG_BUFFER_LEN	512

static void my_func_dbg(char *drv_name, char *func, char *file, int line)
{
	DbgPrint("%s:%s(): File: %s, Line: %d.\n", drv_name, func, file, line);
}

//#define AFT_FUNC_DEBUG()	if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define TDM_FUNC_DBG()		if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define EC_FUNC_DEBUG()		if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define DBG_SET_CFG_FUNC()	if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define DBG_ACUAPI_FUNC()	if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define FUNC_DEBUG()		if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define TDM_FUNC_DBG()		if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define SKB_FUNC()			if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define PROT_FUNC_DEBUG()	if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)
#define TDMAPI_FUNC_DEBUG()	if(0)my_func_dbg(DRIVER_NAME, __FUNCTION__, __FILE__, __LINE__)

#else	/* !__WINDOWS__*/

/* L I N U X */

# define DEBUG_KERNEL(format,msg...)
# define DEBUG_EVENT(format,msg...)
# define DEBUG_MOD(format,msg...)
# define DEBUG_CFG(format,msg...)
# define DEBUG_REG(format,msg...)
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
# define DEBUG_TE3(format,msg...)
# define DEBUG_56K(format,msg...)
# define DEBUG_BRI(format,msg...)
# define DEBUG_PROCFS(format,msg...)
# define DEBUG_TDMV(format,msg...)
# define DEBUG_A600(format,msg...)
# define DEBUG_TEST(format,msg...)
# define DEBUG_DBG(format,msg...)
# define DEBUG_DMA(format,msg...)
# define DEBUG_SNMP(format,msg...)
# define DEBUG_ADD_MEM(a)
# define DEBUG_SUB_MEM(a)
# define DEBUG_RM(format,msg...)
# define DEBUG_HWEC(format,msg...)
# define DEBUG_TDMAPI(format,msg...)
# define DEBUG_FE(format,msg...)
# define DEBUG_NG(format,msg...)
# define WAN_DEBUG_FUNC_START
# define WAN_DEBUG_FUNC_END
# define WAN_DEBUG_FUNC_LINE
# define DEBUG_BRI(format,msg...)
# define DEBUG_BRI_INIT(format,msg...)
# define DEBUG_USB(format,msg...)

#define AFT_FUNC_DEBUG() 

# if (defined __FreeBSD__) || (defined __OpenBSD__) || defined(__NetBSD__)

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
# ifdef WAN_DEBUG_REG
#  undef  DEBUG_REG
#  define DEBUG_REG(format,msg...)		DEBUG_PRINT(format,##msg)
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
# ifdef WAN_DEBUG_TE3
#  undef  DEBUG_TE3
#  define DEBUG_TE3(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_56K
#  undef  DEBUG_56K
#  define DEBUG_56K(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_BRI
#  undef  DEBUG_BRI
#  define DEBUG_BRI(format,msg...)		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_A600
#  undef  DEBUG_A600
#  define DEBUG_A600(format,msg...)		DEBUG_PRINT(format,##msg)
# endif  
# ifdef WAN_DEBUG_PROCFS
#  undef  DEBUG_PROCFS
#  define DEBUG_PROCFS(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TDM_VOICE
#  undef  DEBUG_TDMV
#  define DEBUG_TDMV(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TEST
#  undef  DEBUG_TEST
#  define DEBUG_TEST(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_DBG
#  undef  DEBUG_DBG
#  define DEBUG_DBG(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif

#if 0
# ifdef WAN_DEBUG_MEM
/* This is not used any more */
#  undef  DEBUG_ADD_MEM
#  define DEBUG_ADD_MEM(a)
#  undef  DEBUG_SUB_MEM
#  define DEBUG_SUB_MEM(a)
#endif
#endif

# ifdef WAN_DEBUG_DMA
#  undef  DEBUG_DMA
#  define DEBUG_DMA(format,msg...) 		DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_SNMP
#  undef  DEBUG_SNMP
#  define DEBUG_SNMP(format,msg...) 	DEBUG_PRINT(format,##msg)
# endif
# ifdef WAN_DEBUG_RM
#  undef  DEBUG_RM
#  define DEBUG_RM(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_HWEC
#  undef  DEBUG_HWEC
#  define DEBUG_HWEC(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_TDMAPI
#  undef  DEBUG_TDMAPI
#  define DEBUG_TDMAPI(format,msg...)	DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_FE
#  undef  DEBUG_FE
#  define DEBUG_FE(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_BRI
#  undef  DEBUG_BRI
#  define DEBUG_BRI(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_BRI_INIT
#  undef  DEBUG_BRI_INIT
#  define DEBUG_BRI_INIT(format,msg...)	DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_NG
#  undef  DEBUG_NG
#  define DEBUG_NG(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 
# ifdef WAN_DEBUG_USB
#  undef  DEBUG_USB
#  define DEBUG_USB(format,msg...)		DEBUG_PRINT(format,##msg)
# endif 

#endif	/* __WINDOWS__ */

#if defined (__WINDOWS__)
#define	WAN_KRN_BREAK_POINT()	if(0)DbgBreakPoint()
#else
#define	WAN_KRN_BREAK_POINT()
#endif

#define WAN_DEBUG_FLINE	DEBUG_EVENT("[%s]: %s:%d\n",			\
				__FILE__,__FUNCTION__,__LINE__);

#if defined(WAN_DEBUG_FUNC)
# undef WAN_DEBUG_FUNC_START
# define WAN_DEBUG_FUNC_START	DEBUG_EVENT("[%s]: %s:%d: Start (%d)\n",\
	       		__FILE__,__FUNCTION__,__LINE__, (unsigned int)SYSTEM_TICKS);
# undef WAN_DEBUG_FUNC_END
# define WAN_DEBUG_FUNC_END	DEBUG_EVENT("[%s]: %s:%d: End (%d)\n",	\
	       		__FILE__,__FUNCTION__,__LINE__,(unsigned int)SYSTEM_TICKS);
# undef WAN_DEBUG_FUNC_LINE
# define WAN_DEBUG_FUNC_LINE	DEBUG_EVENT("[%s]: %s:%d: (%d)\n",	\
	       		__FILE__,__FUNCTION__,__LINE__,(unsigned int)SYSTEM_TICKS);

#define BRI_FUNC()	if(0)DEBUG_EVENT("%s(): line:%d\n", __FUNCTION__, __LINE__)
#else
#define BRI_FUNC()
#endif

#define WAN_ASSERT(val) if (val){					\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	WAN_KRN_BREAK_POINT();	\
	return -EINVAL;							\
			}
#define WAN_ASSERT_EINVAL(val) WAN_ASSERT(val)

#define WAN_ASSERT1(val) if (val){					\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	return;								\
			}
#define WAN_ASSERT_VOID(val) WAN_ASSERT1(val)

#define WAN_ASSERT2(val, ret) if (val){					\
	DEBUG_EVENT("************** ASSERT FAILED **************\n");	\
	DEBUG_EVENT("%s:%d - Critical error\n",__FILE__,__LINE__);	\
	return ret;							\
			}
#define WAN_ASSERT_RC(val,ret) WAN_ASSERT2(val, ret)

#define WAN_MEM_ASSERT(str) {if (str){					\
		DEBUG_EVENT("%s: Error: No memory in %s:%d\n",		\
					str,__FILE__,__LINE__);		\
	}else{								\
		DEBUG_EVENT("wanpipe: Error: No memory in %s:%d\n",	\
					__FILE__,__LINE__);		\
	}								\
	}

#define WAN_OPP_FLAG_ASSERT(val,cmd) if (val){				\
	DEBUG_EVENT("%s:%d - Critical error: Opp Flag Set Cmd=0x%x!\n",	\
					__FILE__,__LINE__,cmd);		\
	}


#if defined(__FreeBSD__)
# ifndef WAN_SKBDEBUG
#  define WAN_SKBDEBUG 0
# endif
# define WAN_SKBCRITASSERT(mm) if (WAN_SKBDEBUG){			\
	if ((mm) == NULL){							\
		panic("%s:%d: MBUF is NULL!\n", 				\
					__FUNCTION__,__LINE__);		\
	}									\
	if (((mm)->m_flags & (M_PKTHDR|M_EXT)) != (M_PKTHDR|M_EXT)){	\
		panic("%s:%d: Invalid MBUF m_flags=%X (m=%p)\n",	\
					__FUNCTION__,__LINE__,		\
					(mm)->m_flags,(mm));			\
	}									\
	if ((unsigned long)(mm)->m_data < 0x100){				\
		panic("%s:%d: Invalid MBUF m_data=%p (m=%p)\n",		\
					__FUNCTION__,__LINE__,		\
					(mm)->m_data,(mm));			\
	}									\
}
#else
# define WAN_SKBCRITASSERT(mm)
#endif

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
#define WAN_DEBUG_TE3_MSG		0x02
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

/* WAN DEBUG timer */
#define WAN_DEBUG_INIT(card){						\
	wan_tasklet_t* debug_task = &card->debug_task;			\
	WAN_TASKLET_INIT(debug_task, 0, &wanpipe_debugging, card);	\
	wan_clear_bit(0, (unsigned long*)&card->debug_running);		\
	wanpipe_debug_timer_init(card);					\
	}
#define WAN_DEBUG_END(card){						\
	wan_del_timer(&card->debug_timer);				\
	WAN_TASKLET_KILL(&card->debug_task);				\
	}
#define WAN_DEBUG_STOP(card)	wan_clear_bit(0, &card->debug_running)

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__WINDOWS__)
# define WAN_DEBUG_START(card)						\
		if (!wan_test_bit(0, &card->debug_running)){		\
			wan_set_bit(0, &card->debug_running);		\
			wan_add_timer(&card->debug_timer, 5*HZ);	\
		}
#elif defined(__LINUX__)
# define WAN_DEBUG_START(card)						\
		if (!wan_test_and_set_bit(0, &card->debug_running)){	\
			wan_add_timer(&card->debug_timer, 5*HZ);	\
		}
#else
# error "Undefined WAN_DEBUG_START macro!"
#endif

#if defined(__OpenBSD__) && (OpenBSD >= 200611)
# define WP_READ_LOCK(lock,flag)   {					\
	DEBUG_TEST("%s:%d: RLock %u\n",__FILE__,__LINE__,(u32)lock);	\
	flag = splnet(); }
				     
# define WP_READ_UNLOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: RULock %u\n",__FILE__,__LINE__,(u32)lock);	\
	splx(flag);}

# define WP_WRITE_LOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: WLock %u\n",__FILE__,__LINE__,(u32)lock); 	\
	flag = splnet(); }

# define WP_WRITE_UNLOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: WULock %u\n",__FILE__,__LINE__,(u32)lock); 	\
	splx(flag); }

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__)
# define WP_READ_LOCK(lock,flag)   {					\
	DEBUG_TEST("%s:%d: RLock %u\n",__FILE__,__LINE__,(u32)lock);	\
	flag = splimp(); }
				     
# define WP_READ_UNLOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: RULock %u\n",__FILE__,__LINE__,(u32)lock);	\
	splx(flag);}

# define WP_WRITE_LOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: WLock %u\n",__FILE__,__LINE__,(u32)lock); 	\
	flag = splimp(); }

# define WP_WRITE_UNLOCK(lock,flag) {					\
	DEBUG_TEST("%s:%d: WULock %u\n",__FILE__,__LINE__,(u32)lock); 	\
	splx(flag); }

#elif defined(__WINDOWS__)

# define WP_READ_LOCK(lock,flag)   {							\
	DEBUG_TEST("%s:%d: RLock 0x%p\n",__FILE__,__LINE__,lock);	\
	flag = splimp(); }
				     
# define WP_READ_UNLOCK(lock,flag) {							\
	DEBUG_TEST("%s:%d: RULock 0x%p\n",__FILE__,__LINE__,lock);	\
	splx(flag);}

# define WP_WRITE_LOCK(lock,flag) {								\
	DEBUG_TEST("%s:%d: WLock 0x%p\n",__FILE__,__LINE__,lock); 	\
	flag = splimp(); }

# define WP_WRITE_UNLOCK(lock,flag) {							\
	DEBUG_TEST("%s:%d: WULock 0x%p\n",__FILE__,__LINE__,lock); \
	splx(flag); }

#elif defined(__LINUX__)

# define WAN_TIMEOUT(sec)  { unsigned long timeout; \
	                     timeout=jiffies; \
			     while ((jiffies-timeout)<sec*HZ){ \
				     schedule(); \
			     }\
			   }

# define WP_READ_LOCK(lock,flag)   {  DEBUG_TEST("%s:%d: RLock %u\n",__FILE__,__LINE__,(u32)lock);\
	                             read_lock((lock)); flag=0; }
				     
# define WP_READ_UNLOCK(lock,flag) {  DEBUG_TEST("%s:%d: RULock %u\n",__FILE__,__LINE__,(u32)lock);\
					read_unlock((lock)); flag=0; }

# define WP_WRITE_LOCK(lock,flag) {  DEBUG_TEST("%s:%d: WLock %u\n",__FILE__,__LINE__,(u32)lock); \
	                            write_lock_irqsave((lock),flag); }

# define WP_WRITE_UNLOCK(lock,flag) { DEBUG_TEST("%s:%d: WULock %u\n",__FILE__,__LINE__,(u32)lock); \
	                             write_unlock_irqrestore((lock),flag); }

#else
# error "Undefined WAN_DEBUG_START macro!"
#endif

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


#if defined(__WINDOWS__)
#define DBG_NEWDRV		if(0)DbgPrint
#define FUNC_NEWDRV()	if(0)DbgPrint("%s():Line:%i\n", __FUNCTION__, __LINE__)
#else
#define DBG_NEWDRV		if(1)DEBUG_TEST
#define FUNC_NEWDRV()	if(1)DEBUG_TEST("%s():Line:%i\n", __FUNCTION__, __LINE__)
#endif

static __inline void debug_print_skb_pkt(unsigned char *name, unsigned char *data, int len, int direction)
{
#if defined(__LINUX__) && defined(__KERNEL__)
	int i;
	printk(KERN_INFO "%s: PKT Len(%i) Dir(%s)\n",name,len,direction?"RX":"TX");
	printk(KERN_INFO "%s: DATA: ",name);
	for (i=0;i<len;i++){
		printk("%02X ", data[i]);
	}
	printk("\n");
#endif
}


#if 0

static __inline void debug_print_udp_pkt(unsigned char *data,int len,char trc_enabled, char direction)
{
#if defined(__LINUX__) && defined(__KERNEL__)
	int i,res;
	DEBUG_EVENT("\n");
	DEBUG_EVENT("%s UDP PACKET: ",direction?"RX":"TX");
	for (i=0; i<sizeof(wan_udp_pkt_t); i++){
		if (i==0){
			DEBUG_EVENT("\n");
			DEBUG_EVENT("IP PKT: ");
		}
		if (i==sizeof(struct iphdr)){
			DEBUG_EVENT("\n");
			DEBUG_EVENT("UDP PKT: ");
		}
		if (i==sizeof(struct iphdr)+sizeof(struct udphdr)){
			DEBUG_EVENT("\n");
			DEBUG_EVENT("MGMT PKT: ");
		}
		if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(wan_mgmt_t)){
			DEBUG_EVENT("\n");
			DEBUG_EVENT("CMD PKT: ");
		}
		
		if (trc_enabled){
			if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)){
				DEBUG_EVENT("\n");
				DEBUG_EVENT("TRACE PKT: ");
			}
			if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)+
			       sizeof(wan_trace_info_t)){

				DEBUG_EVENT("\n");
				DEBUG_EVENT("DATA PKT: ");
			}

			res=len-(sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)+sizeof(wan_trace_info_t));
		
			res=(res>10)?10:res;

			if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)+sizeof(wan_trace_info_t)+res){
				break;
			}
			
		}else{
	
			if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)){
				DEBUG_EVENT("\n");
				DEBUG_EVENT("DATA PKT: ");
			}

			res=len-(sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t));
		
			res=(res>10)?10:res;

			if (i==sizeof(struct iphdr)+sizeof(struct udphdr)+
			       sizeof(wan_mgmt_t)+sizeof(wan_cmd_t)+res){
				break;
			}
		}

		DEBUG_EVENT("%02X ",*(data+i));
	}
	DEBUG_EVENT("\n");
#endif
}

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
#if 0
extern void wp_debug_func_init(void);
extern void wp_debug_func_add(unsigned char *func);
extern void wp_debug_func_print(void);
#endif

#endif /* WAN_KERNEL */
#endif /* __WANPIPE_DEBUG_H */
