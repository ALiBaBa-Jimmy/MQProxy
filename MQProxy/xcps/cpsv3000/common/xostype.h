/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xostype.h
**
**  description:  该文件提供通用的类型定义和 常
                        用的宏以及句柄的定义. 所有的平台
                        头文件都必须直接或间接包含此头文件.
**  author: wangzongyou
**
**  date:   2006.7.20
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            

**************************************************************/
#ifndef _XOS_TYPE_H_
#define _XOS_TYPE_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/                  
#include <stdio.h>

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#ifdef XOS_WIN32

typedef char                 XCHAR;
typedef unsigned char       XU8; 
typedef char                XS8;
typedef unsigned short         XU16;
typedef short                 XS16;
typedef unsigned int         XU32;
typedef int                 XS32;
typedef unsigned __int64       XU64;
typedef __int64                XS64;
typedef void                XVOID;

#define XCONST  const
#define XSTATIC static
#define XEXTERN extern
#ifndef XPUBLIC
#define XPUBLIC
#endif

#define XSUCC                 (0)
#define XERROR                 (-1)
/*错误值全部以负值定义，依据各业务层使用情况进行扩充*/

#ifndef XNULL
#define XNULL                (0)
#endif

#define XNULLP              ((void *)0)
#endif /* #ifdef XOS_WIN32 */

#ifdef XOS_LINUX
typedef char                 XCHAR;
typedef unsigned char       XU8; 
typedef char                XS8;
typedef unsigned short         XU16;
typedef short                 XS16;
typedef unsigned int         XU32;
typedef int                 XS32;
typedef unsigned long long         XU64;
typedef long long                 XS64;
#define XVOID void

#define XCONST  const
#define XSTATIC static
#define XEXTERN extern
#ifndef XPUBLIC
#define XPUBLIC
#endif

#define XSUCC                 (0)
#define XERROR                 (-1)
/*错误值全部以负值定义，依据各业务层使用情况进行扩充*/

#ifndef XNULL
#define XNULL                (0)
#endif

#define XNULLP              ((void *)0)

#endif  /*#ifdef XOS_LINUX*/


#ifdef XOS_SOLARIS
typedef char                 XCHAR;
typedef unsigned char       XU8; 
typedef char                XS8;
typedef unsigned short         XU16;
typedef short                 XS16;
typedef unsigned int         XU32;
typedef int                 XS32;
typedef unsigned long         XU64;
typedef long                 XS64;
typedef void                XVOID;

#define XCONST  const
#define XSTATIC static
#define XEXTERN extern
#ifndef XPUBLIC
#define XPUBLIC
#endif

#define XSUCC                 (0)
#define XERROR                 (-1)
/*错误值全部以负值定义，依据各业务层使用情况进行扩充*/

#ifndef XNULL
#define XNULL                (0)
#endif

#define XNULLP              ((void *)0)

#endif  /*#ifdef XOS_SOLARIS*/

#ifdef XOS_VXWORKS

typedef char                 XCHAR;
typedef unsigned char       XU8; 
typedef char                XS8;
typedef unsigned short         XU16;
typedef short                 XS16;
typedef unsigned int         XU32;
typedef int                 XS32;
typedef unsigned long       XU64;
typedef long                XS64;
typedef void                XVOID;


#define XCONST  const
#define XSTATIC static
#define XEXTERN extern
#ifndef XPUBLIC
#define XPUBLIC
#endif

#define XSUCC                 (0)
#define XERROR                 (-1)
/*错误值全部以负值定义，依据各业务层使用情况进行扩充*/

#ifndef XNULL
#define XNULL                (0)
#endif

#define XNULLP              ((void *)0)
#endif /* #ifdef XOS_VXWORKS */

#ifdef XOS_ARCH_64
typedef XU64                XPOINT;
#else
typedef XU32                XPOINT;
#endif

#ifdef XOS_ARCH_64
typedef XU64                XUTIME;
#else
typedef XU32                XUTIME;
#endif

/*64位下文件大小可以超过4G*/
#ifdef XOS_ARCH_64
typedef XU64                XUFILESIZE;
#else
typedef XU32                XUFILESIZE;
#endif

#ifdef XOS_ARCH_64
typedef XS64                XSFILESIZE;
#else
typedef XS32                XSFILESIZE;
#endif

#ifdef XOS_ARCH_64
typedef XU64                XUTIMERPARA;
#else
typedef XU32                XUTIMERPARA;
#endif



#define BLANK_UCHAR             ((XU8)0xFF)
#define BLANK_USHORT            ((XU16)0xFFFF)
#define BLANK_ULONG             ((XU32)0xFFFFFFFF)

typedef enum
{
    XFALSE=0,
    XTRUE  
}XBOOL;

/* 句柄声明*/
#ifndef XOS_DECLARE_HANDLE
#define XOS_DECLARE_HANDLE(name)    typedef struct { int unused; } name##__ ; \
                typedef const name##__ * name; \
                typedef name*  LP##name
#endif

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/
/*ip 地址定义*/
typedef struct
{
    XU32 ip;
    XU16 port;
}t_IPADDR;

/*sctp 地址结构t_SCTPIPADDR支持的IP数量*/
#define SCTP_ADDR_NUM       8
/*SCTP ip 地址定义*/
typedef struct
{
    XU32 ip[SCTP_ADDR_NUM];
    XU16 port;
    XU16 ipNum;
}t_SCTPIPADDR;

/*消息优先级*/
typedef enum
{
    eMinPrio        = 0,
    eChuckMsgPrio   = 1,   /*可丢弃的消息优先级；*/
    eNormalMsgPrio  = 2,   /* 一般消息优先级；*/
    eFastMsgPrio    = 3,   /*快速消息优先级；*/
    eAdnMsgPrio     = 4,   /*紧急消息优先级；*/
    eStandbyPrio    = 5,   /*备用消息优先级；*/
    eTimePrio       = 6,   /*定时器消息优先级；*/
    eHAPrio         = 7,   /*HA消息优先级；*/
    eMAXPrio
}e_MSGPRIO;

/*消息头相关的定义*/
typedef struct _XOSMSGHEAD
{
    XU32  PID;  /*处理器号*/
    XU32  FID;  /*功能块号*/
    XU32  FsmId;/* 内部连接号*/
}t_XOSUSERID;

typedef struct  _XOSCOMMGHEAD
{
    t_XOSUSERID datasrc;  /*消息来源,如FID全局唯一，填写FID即可*/
    t_XOSUSERID datadest; /*消息目的*/
    XU16       msgID;           /*消息类型*/
    XU16       subID;           /*消息子类型*/  
    e_MSGPRIO  prio;             /*消息优先级*/   
    XU32    traceID;           /*TraceID成员字段*/
    XU32    logID;          /*LogID成员字段*/
    XU32       length;          /*消息长度*/   
    XVOID      *message;        /*消息指针*/ 
} t_XOSCOMMHEAD;

typedef enum
{
   PL_MIN = 0,
   PL_DBG,        /*很低的打印级别(调试型打印)*/
   PL_INFO,       /*调试信息打印级别(调试型打印)*/
   PL_WARN,       /*运行信息打印级别(运行打印)*/
   PL_ERR,        /*一般性错误打印级别(错误型打印)*/
   PL_EXP,        /*异常打印级别(错误型打印)*/
   PL_LOG,        /*必须打印到日志文件中的信息*/
   PL_MAX
}e_PRINTLEVEL;
/*-------------------------------------------------------------------------
API 声明
-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/

