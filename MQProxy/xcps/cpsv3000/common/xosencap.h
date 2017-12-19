/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xos.h
**
**  description:  给予业务层提供所有的平台接口封装 
**
**  author: wulei
**
**  date:   2006.3.7
**
***************************************************************
**                          history                     
**  
***************************************************************
**   author          date              modification            
**   wulei         2006.3.7              create  
**************************************************************/
#ifndef _XOS_H_
#define _XOS_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*------------------------------------------------------------------------
                  包含头文件
-------------------------------------------------------------------------*/
#ifdef XOS_WIN32
#include "winos.h"
#endif

#ifdef XOS_LINUX
#include "linos.h"
#endif

#ifdef XOS_SOLARIS
#include "solos.h"
#endif

#ifdef XOS_VXWORKS
#include "vxos.h"
#endif

#include "xostype.h"
#include "xosmmgt.h"
#include <ctype.h>

/*-------------------------------------------------------------------------
                宏定义
-------------------------------------------------------------------------*/
#define MAXNUMSTRLEN 11

#define XOS_INET_ADDR_LEN (18)



#define XOS_Malloc(size)    malloc((size_t)(size))
#define XOS_Free(pt)\
{\
    if(XNULL == pt)\
    {\
        printf("free null is error\n");\
    }\
    else\
    {\
        free(pt);\
    }\
}

#ifdef XOS_DEBUG
#define XOS_Assert( exp ) assert((XS32)(exp) )
#endif

#ifdef XOSUNUSED
#undef XOSUNUSED
#define XOS_UNUSED(x) (x=x)
#else
#define XOS_UNUSED(x) (x=x)
#endif


#define XOS_GetChar()     OS_GetChar()


/* 字符的比较*/
#define XOS_StrCmp(s1, s2)    strcmp((char *)s1,(char *)s2)   /*#不提倡使用*/
#define XOS_StrNcmp(s1,s2,n)        strncmp((char *)s1,(char *)s2,(size_t)(n))

/* 字符的拷贝*/
#define XOS_StrCpy(dest,src)    strcpy((char *)dest , (char *)src)   /*#不提倡使用*/
#define XOS_StrNcpy(dest, src,n )    strncpy((char *)dest,(char *)src,(size_t)(n) )

/*字符串连接*/
#define XOS_StrCat(dest, src)   strcat((char *)dest, (XCONST XCHAR *)src)   /*#不提倡使用*/
#define XOS_StrNCat(dest,src,n) strncat((XCHAR *)dest, (XCONST XCHAR *)src,(size_t)(n))

/* 取字符长度*/
#define XOS_StrLen(s)    strlen((char *)s)


/* 字符的查找*/
#define XOS_StrChr(s, c)    strchr((char *)s,(int)(c))
#define XOS_StrStr(haystack,needle)    strstr((char *)haystack,(char *)needle)

/* 内存的拷贝，赋值*/
#define XOS_MemCpy(dest,src,n)    memcpy(dest,src,n)
#define XOS_MemMove(dest,src,n)  memmove(dest,src,n)
#define XOS_MemSet(dest,src,n)    memset(dest,src,n)
#define XOS_MemCmp(dest,src,n)  memcmp(dest,src,n)

/* 字符大小写转换*/
#define XOS_ToLower( c )    tolower( c )
#define XOS_ToUpper( c )    toupper( c )

/* 字节序转换*/
/* 2 字节网络序转主机序*/
#define XOS_NtoHs(netshort)    ntohs((unsigned short int)(netshort))    

/* 2 字节主机序转网络序*/
#define XOS_HtoNs(hostshort)    htons((unsigned short int)(hostshort))    

/* 4 字节网络序转主机*/
#define XOS_NtoHl(netlong)    ntohl((unsigned long int)(netlong))    

/* 4 字节主机序转网络序*/
#define XOS_HtoNl(hostlong)    htonl((unsigned long int)(hostlong))    

/* 取最大、最小值*/
#define XOS_MAX(x,y) ((x)>=(y)?(x):(y))
#define XOS_MIN(x,y) ((x)>=(y)?(y):(x))


/* 数字转换成字符*/

/* 交换两个变量的值*/

/* 高低位转换*/


/*位操作的宏申明*/
#define XOS_GetHiByte(w)   (((XU16)(w) >> 8) & 0xff)      /* get hi byte from word */
#define XOS_GetLoByte(w)   ((XU16)(w) & 0xff)             /* get lo byte from word */
#define XOS_GetHiWord(l)   (((XU32)(l) >> 16) & 0xffffL)  /* get hi word of long */
#define XOS_GetLoWord(l)   ((XU32)(l) & 0xffffL)          /* get lo word of long */

/* envind_h_001_102: add 64 bit support */
#if (defined(ALPHA) || defined(BIT_64))
#define XOS_GetLo32Bit(l) ((U64)(l) & 0xffffffffL) /*get lo 32 bits */
#define XOS_GetHi32Bit(l) (((U64)(l) >> 32) & 0xffffffffL) /*get hi 32 bits */
#endif

#define XOS_PutHiByte(w,b) (XU16) (((XU16)(b) << 8) | ((XU16)(w) & 0x00ff))      /* put hi byte to word */
#define XOS_PutLoByte(w,b) (XU16) (((XU16)(b) & 0xff) | ((XU16)(w) & 0xff00))             /* put lo byte to word */
#define XOS_PutHiWord(l,w) (XU32) (((XU32)(w) << 16) | ((XU32)(l) & (XU32)0x0000ffff)) /* put hi word to long */
#define XOS_PutLoWord(l,w) (XU32) (((XU32)(w) & 0xffff) | ((XU32)(l) & (XU32)0xffff0000))         /* put lo word to long */

/* envind_h_001_102: add 64 bit support */
#if (defined(ALPHA) || defined(BIT_64))
#define PutLo32Bit(l,w) (U64) (((U64)(w) & 0xffffffff) | ((U64)(l) & (U64)0xffffffff00000000))  /* put lo 32 bits */
#define PutHi32Bit(l,w) (U64) (((U64)(w) << 32) | ((U64)(l) & (U64)0x00000000ffffffff))         /* put hi 32 bits */
#endif


/************************************************************************
功能：produce output according to a format write to the character string str
输入：
char *str  : 输出buf
size_t n   : buf允许最大字符个数
char *fmt  : 格式字符串 
va_list ap : 可变参
输出：
char *str  : 输出buf
返回：返回实际获取字符个数
说明：仅仅是vxworks平台下接口
************************************************************************/
#ifdef XOS_VXWORKS 
#define vx_vsnprintf ruby_vsnprintf

#endif

#ifdef XOS_LINUX

/************************************************************************
函数名: XOS_ExeCmd
功能：调用命令行执行
输入：无
输出： 
返回：
XSUCC  -   成功
XERROR -   失败
说明：
************************************************************************/
XPUBLIC XS32 XOS_ExeCmd(XCHAR *pCmd, XCHAR *pBuf, XU32 len);

XPUBLIC XS32 XOS_System(const XCHAR * pCommand);

/************************************************************************
函数名: XOS_ExeCmdRetVal
功能: 执行shell命令，获取被执行命令的返回值
输入:
输出:
返回: 返回被执行命令的返回值
说明: -1: fork失败
      -2: system执行命令失败
      其他: 被执行命令的返回值，为非负数 0-127
************************************************************************/
XPUBLIC XS32 XOS_ExeCmdRetVal(XCHAR *pCmd);

/************************************************************************
函数名:XOS_ExeCmdByPopen_Ex
功能:  通过popen执行命令行,返回成功与否，并获取结果内容。
输入:  pCmd-命令行
       pBuf-保存返回结果的内容
       len-缓存pBuf的可用长度
       ms - popen后，获取命令结果之前，需要delay的时间，单位毫秒
输出:
返回: 成功返回命令执行的结果,否则返回XERROR
说明: 此API不会导致文件继承
************************************************************************/
XPUBLIC XS32 XOS_ExeCmdByPopen_Ex(XCHAR *pCmd, XCHAR *pBuf, XU32 len, XU32 ms);



/************************************************************************
函数名:    XOS_GetSysRunTime
功能：获取系统运行时间(精确到秒)
输入
输出：pSecs -   当前系统运行时间（秒）
返回：
XSUCC    -    成功
XERROR    -    失败
说明：返回的是系统运行至今的秒数
************************************************************************/
XS32 XOS_GetSysRunTime(XU64 *pSecs);

/************************************************************************
函数名: XOS_GetCpuCoreNum
功能：获取cpu核数
输入：
输出：
返回：返回cpu核数
说明：
************************************************************************/
XS32 XOS_GetCpuCoreNum(XVOID);

/************************************************************************
函数名: XOS_SetThreadAffinity
功能：设置线程级cpu绑定
输入：pCpuId : cpu号的数组
      usCpuNum 数组元素个数 
输出：
返回：XSUCC - 成功 XERROR  -  失败
说明：由各个线程调用, 函数调用的所处位置决定了是绑定所有线程还是当前线程:
1，如果是在主线程中调用，会使主线程和之后创建的所有线程绑定有效
2，如果是在子线程中调用，只对当前线程有效\
************************************************************************/
XS32 XOS_SetThreadAffinity(const XU16 *pCpuId, XS32 CpuNum);

/************************************************************************
函数名: XOS_SetThreadAffinity_t
功能：设置线程级cpu绑定, 可指定绑定线程的线程id
输入：thread : 需要绑定cpu的线程id
      pCpuId : cpu号的数组
      usCpuNum 数组元素个数 
输出：
返回：XSUCC - 成功 XERROR  -  失败
说明：
************************************************************************/
XS32 XOS_SetThreadAffinity_t(pthread_t thread,const XU16 *pCpuId, XS32 CpuNum);

/************************************************************************
 函数名: XOS_GetLogicPid
 功能: 获取当前进程的逻辑进程号
 输入:
 输出:
 返回: 成功返回非负数, 否则返回XERROR
 说明: 只针对业务网元使用
************************************************************************/
XS32 XOS_GetLogicPid(XVOID);

/************************************************************************
 函数名: XOS_GetNeId
 功能: 获取当前进程的网元id
 输入:
 输出:
 返回: 成功返回非负数, 否则返回XERROR
 说明: 只针对业务网元使用
************************************************************************/
XS32 XOS_GetNeId(XVOID);

/************************************************************************
 函数名: XOS_GetWorkspaceId
 功能: 获取当前进程的工作区id
 输入:
 输出:
 返回: 成功返回非负数, 否则返回XERROR
 说明: 只针对业务网元使用
************************************************************************/
XS32 XOS_GetWorkspaceId(XVOID);

XS32 XOS_GetNeTypeId(XVOID);
XS8* XOS_GetNeTypeStr(XVOID);
XS8* XOS_GetProcTypeStr(XVOID);

XS32 XOS_GetTsHighSlot(XVOID);

XS32 XOS_GetTsLowSlot(XVOID);

#endif

/************************************************************************
功能：convert a network address to dotted decimal notation
输入：
inetAddress  -   要转换的in_addr
pString      -   用来保存返回的点分十进制IP
输出：
pString      -   点分十进制IP
返回：
说明：pString为长度16的字符数组，即XCHAR pString[16]
************************************************************************/
#ifdef XOS_VXWORKS 
#define XOS_Inet_ntoa(naddr,straddr)    inet_ntoa_b(naddr,straddr)
#else 
#define XOS_Inet_ntoa(naddr,straddr) { \
    register char *__p = (char *)&naddr; \
    sprintf(straddr, "%d.%d.%d.%d", \
                __p[0]&0xff, __p[1]&0xff, __p[2]&0xff, __p[3]&0xff); \
} 
#endif 

/*-------------------------------------------------------------------------
                结构和枚举声明
-------------------------------------------------------------------------*/
/*任务优先级枚举定义*/
typedef enum
{
#ifdef XOS_WIN32
    TSK_PRIO_LOWEST=THREAD_PRIORITY_LOWEST+2,
    TSK_PRIO_LOWER=THREAD_PRIORITY_BELOW_NORMAL+2,
    TSK_PRIO_NORMAL=THREAD_PRIORITY_NORMAL+2,
    TSK_PRIO_HIGHER=THREAD_PRIORITY_ABOVE_NORMAL+2,
    TSK_PRIO_HIGHEST=THREAD_PRIORITY_HIGHEST+2
#endif    

#ifdef XOS_LINUX
    TSK_PRIO_LOWEST=60,
    TSK_PRIO_LOWER=65,
    TSK_PRIO_NORMAL=70,
    TSK_PRIO_HIGHER=75,
    TSK_PRIO_HIGHEST=80
#endif

#ifdef XOS_SOLARIS  /*solaris 的优先级由低到高为1~59*/
    TSK_PRIO_LOWEST=30,
    TSK_PRIO_LOWER=35,
    TSK_PRIO_NORMAL=40,
    TSK_PRIO_HIGHER=45,
    TSK_PRIO_HIGHEST=50
#endif

#ifdef XOS_VXWORKS
    TSK_PRIO_LOWEST=200,
    TSK_PRIO_LOWER=100,
    TSK_PRIO_NORMAL=80,
    TSK_PRIO_HIGHER=70,
    TSK_PRIO_HIGHEST=60
#endif
}e_TSKPRIO;

typedef struct _xos_timeb
{
    XUTIME     time;          /*公元1970.1.1至今的秒数*/
    XU16     millitm;       /*毫秒数*/
    XS16     timezone;      /*目前时区和UTC相差的时间，单位为分钟*/
}t_XOSTB;

#if 0
typedef struct _xos_timeb
{
    XU32     time;          /*公元1970.1.1至今的秒数*/
    XU16     millitm;       /*毫秒数*/
    XS16     timezone;      /*目前时区和UTC相差的时间，单位为分钟*/
    XS16     dstflag;       /*dstflag  is  a  flag  that,  if nonzero, indicates that Daylight Saving 
                                    time applies locally during the appropriate part of the year. */
}t_XOSTB;
#endif

typedef struct _sys_td
{
    XU32     dt_sec;         /* seconds */
    XU32     dt_min;         /* minutes */
    XU32     dt_hour;        /* hours */
    XU32     dt_mday;        /* day of the month */
    XU32     dt_mon;         /* month */
    XU32     dt_year;        /* year */
    XU32     dt_wday;        /* day of the week */
    XU32     dt_yday;        /* day in the year */
    XU32     dt_isdst;       /* daylight saving time */ 
}t_XOSTD;

#ifndef XOS_SOLARIS
typedef time_t t_XOSTT; /**/
typedef clock_t t_XOSCT; 
#else
#ifndef XOS_ARCH_64
    typedef XU32 t_XOSTT; /**/
    typedef XU32 t_XOSCT; 
#else
    typedef XU64 t_XOSTT; /**/
    typedef XU64 t_XOSCT; 

#endif

#endif

#ifdef XOS_VXWORKS
typedef struct
{
    XU8 ucWeekDay;
    XU8 ucDay;
    XU8 ucMonth;
    XU8 ucYear;
    XU8 ucSecond;
    XU8 ucMinute;
    XU8 ucHour;
}XOS_SERTIME_T;

#endif

typedef struct _os_sysinfo
{
    XU32 dwNumberOfProcessors;   /* 1.CUP个数*/
    XU32 dwProcessorLoad;    /* 2.CUP占用率*/
    t_XOSTD localtime;   /* 5.核心时间*/
    
    XU32 dwMemoryLoad;     /* 1.内存占用率*/
    XU32 dwTotalPhys;    /* 2.物理内存总额*/
    XU32 dwAvailPhys;    /* 3.物理内存可用*/
    XU32 dwTotalPageFile;    /* 4.交换区总额*/
    XU32 dwAvailPageFile;   /* 5.交换区可用*/

    XU32 dwtotalthreads;    /* 1.多少个进程*/
    XU32 dwthreadRun;    /* 2.多少个进程运行中(linux中可用)*/
    XU32 dwthreadssleep;    /* 3.多少个进程睡眠中(linux中可用)*/
}t_XOSSYSINFO;

#define MAX_LOCALIP_NUM    16
typedef struct _LOCALIP_LIST
{
    XU32  nIPNum;
    XU32  localIP[MAX_LOCALIP_NUM];
} t_LOCALIPLIST;


/*-------------------------------------------------------------------------
                API 声明
-------------------------------------------------------------------------*/

/************************************************************************
函数名:    XOS_StrtoIp
功能：将网络地址转换成网络二进制的数字
输入：
pString  -   要转换的网络地址
inetAddress - 用来保存返回的网络二进制的数字
输出：inetAddress - 网络二进制的数字
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_StrtoIp( XCHAR *pString , XU32 *inetAddress );


/************************************************************************
函数名:    XOS_StrNtoIp
功能：将网络地址转换成网络二进制的数字，输入的网络地址字符串若不按'\0'结束，
      则只按前len个字符进行处理;
      例如：合法的输入"168.0.2","168.0.2."
输入：
pString  -   要转换的网络地址、
inetAddress - 用来保存返回的网络二进制的数字
len    -    网络地址字符串pString的长度
输出：inetAddress - 网络二进制的数字
返回值：
XSUCC    -    成功
XERROR    -    失败
说明：各操作系统下存在差异：比如"202.0.2."在Windows、Linux下被认为是不符合
      格式的IP地址，但在Solaris和vxWork下却能成功转换；所以，为了
      不出现使用上的问题，请务必以"168.0.2.258"的格式输入IP。
************************************************************************/
XS32 XOS_StrNtoIp ( XCHAR *pString , XU32 *inetAddress , XU32 len);


/************************************************************************
函数名:    XOS_IptoStr
功能：将网络二进制的数字转换成网络地址
输入：
inetAddress  -   要转换的网络二进制数字
pString - 用来保存返回的点分十进制IP
输出：pString - 点分十进制IP
返回：
XSUCC    -    成功
XERROR    -    失败
说明：pString为长度16的字符数组，即XCHAR pString[16]
************************************************************************/
XS32 XOS_IptoStr( XU32 inetAddress , XCHAR *pString );


/************************************************************************
函数名:    XOS_IpNtoStr
功能：将网络二进制的数字转换成网络地址并写入pString里（只写入前len个字符或遇'\0'止）
输入：
inetAddress  -   要转换的网络二进制数字（注意不能超出范围）
pString - 用来保存转换后IP字符串的缓冲区（必须为可写的）
len    -    缓冲区pString的长度
输出：pString - 点分十进制IP
返回值：
XSUCC    -    成功
XERROR    -    失败
说明：一般pString为长度16的字符数组空间，即XCHAR pString[16];
注：1.由于系统函数inet_ntoa的缘故，若两个线程同时调用此函数，
      在第一个的返回值还没保存时可能会被第二个的返回值给覆盖掉
    2.VXWORKS下调用的系统函数为 inet_ntoa_b ，因为inet_ntoa ：
    Each time this routine is called, 18 bytes are allocated from memory.  
    
************************************************************************/
XS32 XOS_IpNtoStr ( XU32 inetAddress , XCHAR *pString , XU32 len);


/************************************************************************
函数名:    XOS_StrNtoXU32
功能：将字符串转换成长整型数（只转换前len个字符）;
输入：
string    -    要转换的字符串
len    -    字符串string的长度
输出：value    -    转换得到的长整型数
返回：
XSUCC    -    成功
XERROR    -    失败
说明：转换时会跳过前面的空格字符，直到遇到数字时才开始转换，
      到再次遇到非数字或字符串结束符'\0'时返回。
************************************************************************/
XS32 XOS_StrNtoXU32( XCHAR *string , XU32 *value ,XU32 len );


/************************************************************************
函数名:    XOS_XU32NtoStr
功能：将无符号长整型数转换成字符串（size够长就写到最后加入'\0'止，否则只写入size个）
输入：
value    -    要转换的无符号整型
str    -    用来保存转换后的字符串
size    -    字符串的长度
输出：str    -    转换得到的字符串
返回值：
XSUCC    -    成功
XERROR    -    失败
说明: windows 下是ultoa，LINUX下用snprintf
************************************************************************/
XS32 XOS_XU32NtoStr(XU32 value, XCHAR *str , XU32 size );


/************************************************************************
函数名:    XOS_GetSysTime
功能：获取系统时间(精确到千分之一秒)
输入：timex -   用来保存时间的结构体
输出：timex -   当前系统时间（秒,毫秒及 目前时区和UTC相差的时间，单位为分钟）
返回：
XSUCC    -    成功
XERROR    -    失败
说明：返回的是公元1970.1.1至今的秒数与毫秒数
      vxWorks下返回的是板子起来的秒数
注：vxWorks下，若传入正确的初始时间，则可以获取当前的真实时间            
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex );


#if 0
/************************************************************************
函数名:    XOS_GetSysInfo
功能：获取系统信息
输入：N/A
输出：systeminfo    -    指向系统信息结构体的指针
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_GetSysInfo(t_XOSSYSINFO *systeminfo);
#endif

/************************************************************************
函数名:    XOS_MutexCreate
功能：互斥量的初始化
输入：mutex    -    互斥量ID
输出：N/A
返回：
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexCreate(t_XOSMUTEXID *mutex );


/************************************************************************
函数名:    XOS_MutexLock
功能：获得互斥量
输入：mutex    -    互斥量ID
输出：N/A
返回：
XSUCC        -    成功
XERROR    -    失败
说明：必须是在同一进程的不同线程间使用;
      若在同一线程内可以多次lock（但最好与XOS_MutexUnlock成对出现）
************************************************************************/
XS32 XOS_MutexLock(t_XOSMUTEXID *mutex);


/************************************************************************
函数名:    XOS_MutexUnlock
功能：互斥量解锁
输入：mutex    -    互斥量ID
输出：N/A
返回：
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexUnlock(t_XOSMUTEXID *mutex);


/************************************************************************
函数名:    XOS_MutexDelete
功能：互斥量销毁
输入：mutex    -    互斥量ID
输出：N/A
返回：
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_MutexDelete(t_XOSMUTEXID *mutex);

/************************************************************************
函数名:    XOS_SemCreate
功能:计数信号量(无名)
参数:
Input:
initNum    -    计数信号量的初值
name      -      信号量名字
output:    semaphore   - 信号量id
Return:
XSUCC    -    成功
XERROR    -    失败
说明：vxWorks下支持的最大值可以是XU32的最大值；
      而Windows、linux、Solaris只能支持XU32最大值的一半(2147483647?)
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum );


/************************************************************************
函数名:    XOS_SemGet
功能:获得信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SemGet(t_XOSSEMID *semaphore);

/************************************************************************
函数名:    XOS_SemGetExt
功能:获得信号量
参数:
Input:    semaphore    -    信号量的id
          timeout   超时时间，单位 秒
Return:
XSUCC    -    成功
XERROR    -    失败
说明：  
************************************************************************/
XS32 XOS_SemGetExt(t_XOSSEMID *semaphore, XS32 timeout);

/************************************************************************
函数名:    XOS_SemPut
功能:释放信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SemPut(t_XOSSEMID *semaphore);


/************************************************************************
函数名:    XOS_SemDelete
功能:删除信号量
参数:
Input:    semaphore    -    信号量的id
Return:
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore);


/************************************************************************
函数名:    XOS_TaskCreate
功能:创建任务
参数:
Input:
pTaskName    -    任务名
iTaskPri    -    任务优先级
iTaskStackSize    -    堆栈大小
fTaskFun    -    任务处理函数
pPar    -    参数
Output:    idTask    -    任务标识
Return:
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_TaskCreate(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask);



/************************************************************************
函数名: XOS_TaskCreate_Ex
功能  : 创建任务
Input:
        pTaskName        -    任务名
        iTaskPri        -    任务优先级
        iTaskStackSize    -    堆栈大小
        fTaskFun        -    任务处理函数
        pPar            -    参数      
        pOwnPar :       -    创建函数内部使用的，目前用来传递cpu绑定信息
Output:    
        idTask            -    任务标识
Return:
        XSUCC        -    成功
        XERROR        -    失败
说明  ：比XOS_TaskCreate多了个参数pOwnPar
************************************************************************/
XS32 XOS_TaskCreate_Ex(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask, XVOID *pOwnPar);




/************************************************************************
函数名:    XOS_TaskDel
功能:删除任务(请慎用)
参数:
Input:    idTask    -    任务标识
Output:    N/A
Return:
XOS_SUCC    -    成功
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 XOS_TaskDel(t_XOSTASKID idtask);


#if 0
/************************************************************************
函数名:    CM_Newkey
功能:新建一个键
参数:
Input:     N/A    
Return:
成功    -    指向一个键值的指针
失败    -    XOS_ERROR
说明：
************************************************************************/
 XVOID    *CM_Newkey(XVOID);    /*此3个函数内部可能会使用，现为空，需要时再加入 mod by wentao*/


/************************************************************************
函数名:    CM_Setkey
功能:将线程数据和一个键绑定在一起
参数:
Input:
key    -    指向一个键值的指针
pointer    -    指向要绑定的数据结构的指针
Return:
成功    -    XOS_SUCC
失败    -    XOS_ERROR
说明：
************************************************************************/
 XU32 CM_Setkey(XVOID *key, XCONST XVOID *pointer);


/************************************************************************
函数名:    CM_Getkey
功能:取得键绑定的线程数据
参数:
Input:     key    -    指向一个键值的指针
Return:
成功    -    返回键绑定的数据(非0)    
失败    -    XOS_ERROR
说明：
************************************************************************/
 XVOID *CM_Getkey(XVOID *key);
#endif


/************************************************************************
函数名:    XOS_SusPend
功能：挂起任务
输入：N/A
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_SusPend( XVOID );

/************************************************************************
函数名:    XOS_Sleep
功能：任务睡眠一段时间
输入：ms    -   时间长度(million second)
输出：N/A
返回：
XSUCC    -    成功
XERROR    -    失败
说明：
************************************************************************/
XS32 XOS_Sleep(XU32 ms);

/************************************************************************
函数名: XOS_GetLocalIP
功能: 获取本机 IP地址
输入: 无
输出:
pLocalIPList.nIPNum; 获取的IP 的个数, 
pLocalIPList.localIP[MAX_LOCALIP_NUM]; 32 位 IP .
eg. pLocalIPList.localIP[0] 是第一个本机IP地址.
返回: 本地 IP 地址
说明: 
************************************************************************/
XS32 XOS_GetLocalIP( t_LOCALIPLIST* pLocalIPList);


#if 0
/************************************************************************
函数名:    XOS_Assert
功能：表达式结果正确性测试并可使程序中止
输入：exp    -    需要判断的表达式;
输出：
返回值：如果括号中的逻辑表达式值为假的话，会提示具体在哪个文件的哪一行发生了断言错误;
说明：
************************************************************************/
XVOID XOS_Assert(XS32 exp );
#endif


#if 0
/************************************************************************
函数名:    CM_GetFilePath
功能：  得到文件路径
输入：
filename - 文件名字
buffer   - 装路径名的缓冲区
len      - 装路径名的缓冲区长度
输出：N/A
返回：
成功 － 实际需要的缓冲区长度
XOS_ERROR    -    失败
说明：
************************************************************************/
XS32 CM_GetFilePath(XCHAR *filename, XCHAR *buffer, XU32 len);
#endif
/************************************************************************
 函数名: XOS_GetPID( XVOID )
 功能:   获取当前平台的 PID
 输入:   无
 输出:   当前平台的 PID
 返回:
 说明: 此函数为公用，定义在 xosmodule.c 内
************************************************************************/
XPUBLIC XU32 XOS_GetLocalPID( XVOID );

XVOID XOS_SetLocalPID(XU32 localpid);


/************************************************************************
函数名: XOS_Root
功能：  平台软件入口函数
输入：  XVOID
输出：  N/A
返回：  XSUCC OR XERROR
说明：  进行平台软件的初始化，并根据业务层填写的注册结构进行任务
        的创建，调度等（定义在 xosroot.c 内）
************************************************************************/
XPUBLIC XS8 XOS_Root(XVOID);

XPUBLIC XS32 XOS_Reset(XU32 type);


XS32 XOS_SetSysTime( t_XOSTB *timex );

XS32 XOS_GetCpuRate(int *rate);

#ifdef XOS_ARCH_64
XS32 XOS_GetMemInfo(XU64* kbAlloc, XU64* kbTotal);
#else
XS32 XOS_GetMemInfo(XS32* kbAlloc, XS32* kbTotal);
#endif

#ifdef XOS_VXWORKS
extern STATUS drv_GetCpuRate(int* rate);
#endif

/************************************************************************
函数名:XOS_Strerror
功能:  将传入的errno，转换为对应的错误信息字符串
输入:  errnum  错误码
输出:  
返回: success返回传入的errmsg的地址，fail返回"unknown errno!";
说明: 此API支持所有平台
************************************************************************/
XCHAR* XOS_StrError(XS32 errnum);

/************************************************************************
函数名: XOS_StackDump
功能：  将函数调用栈信息记录到dump文件
输入：  fid           - 功能块id
输出：  N/A
返回：  XVOID 
说明： 
************************************************************************/
XVOID XOS_StackDump(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/

