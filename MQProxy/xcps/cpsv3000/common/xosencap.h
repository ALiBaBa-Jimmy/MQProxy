/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xos.h
**
**  description:  ����ҵ����ṩ���е�ƽ̨�ӿڷ�װ 
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
                  ����ͷ�ļ�
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
                �궨��
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


/* �ַ��ıȽ�*/
#define XOS_StrCmp(s1, s2)    strcmp((char *)s1,(char *)s2)   /*#���ᳫʹ��*/
#define XOS_StrNcmp(s1,s2,n)        strncmp((char *)s1,(char *)s2,(size_t)(n))

/* �ַ��Ŀ���*/
#define XOS_StrCpy(dest,src)    strcpy((char *)dest , (char *)src)   /*#���ᳫʹ��*/
#define XOS_StrNcpy(dest, src,n )    strncpy((char *)dest,(char *)src,(size_t)(n) )

/*�ַ�������*/
#define XOS_StrCat(dest, src)   strcat((char *)dest, (XCONST XCHAR *)src)   /*#���ᳫʹ��*/
#define XOS_StrNCat(dest,src,n) strncat((XCHAR *)dest, (XCONST XCHAR *)src,(size_t)(n))

/* ȡ�ַ�����*/
#define XOS_StrLen(s)    strlen((char *)s)


/* �ַ��Ĳ���*/
#define XOS_StrChr(s, c)    strchr((char *)s,(int)(c))
#define XOS_StrStr(haystack,needle)    strstr((char *)haystack,(char *)needle)

/* �ڴ�Ŀ�������ֵ*/
#define XOS_MemCpy(dest,src,n)    memcpy(dest,src,n)
#define XOS_MemMove(dest,src,n)  memmove(dest,src,n)
#define XOS_MemSet(dest,src,n)    memset(dest,src,n)
#define XOS_MemCmp(dest,src,n)  memcmp(dest,src,n)

/* �ַ���Сдת��*/
#define XOS_ToLower( c )    tolower( c )
#define XOS_ToUpper( c )    toupper( c )

/* �ֽ���ת��*/
/* 2 �ֽ�������ת������*/
#define XOS_NtoHs(netshort)    ntohs((unsigned short int)(netshort))    

/* 2 �ֽ�������ת������*/
#define XOS_HtoNs(hostshort)    htons((unsigned short int)(hostshort))    

/* 4 �ֽ�������ת����*/
#define XOS_NtoHl(netlong)    ntohl((unsigned long int)(netlong))    

/* 4 �ֽ�������ת������*/
#define XOS_HtoNl(hostlong)    htonl((unsigned long int)(hostlong))    

/* ȡ�����Сֵ*/
#define XOS_MAX(x,y) ((x)>=(y)?(x):(y))
#define XOS_MIN(x,y) ((x)>=(y)?(y):(x))


/* ����ת�����ַ�*/

/* ��������������ֵ*/

/* �ߵ�λת��*/


/*λ�����ĺ�����*/
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
���ܣ�produce output according to a format write to the character string str
���룺
char *str  : ���buf
size_t n   : buf��������ַ�����
char *fmt  : ��ʽ�ַ��� 
va_list ap : �ɱ��
�����
char *str  : ���buf
���أ�����ʵ�ʻ�ȡ�ַ�����
˵����������vxworksƽ̨�½ӿ�
************************************************************************/
#ifdef XOS_VXWORKS 
#define vx_vsnprintf ruby_vsnprintf

#endif

#ifdef XOS_LINUX

/************************************************************************
������: XOS_ExeCmd
���ܣ�����������ִ��
���룺��
����� 
���أ�
XSUCC  -   �ɹ�
XERROR -   ʧ��
˵����
************************************************************************/
XPUBLIC XS32 XOS_ExeCmd(XCHAR *pCmd, XCHAR *pBuf, XU32 len);

XPUBLIC XS32 XOS_System(const XCHAR * pCommand);

/************************************************************************
������: XOS_ExeCmdRetVal
����: ִ��shell�����ȡ��ִ������ķ���ֵ
����:
���:
����: ���ر�ִ������ķ���ֵ
˵��: -1: forkʧ��
      -2: systemִ������ʧ��
      ����: ��ִ������ķ���ֵ��Ϊ�Ǹ��� 0-127
************************************************************************/
XPUBLIC XS32 XOS_ExeCmdRetVal(XCHAR *pCmd);

/************************************************************************
������:XOS_ExeCmdByPopen_Ex
����:  ͨ��popenִ��������,���سɹ���񣬲���ȡ������ݡ�
����:  pCmd-������
       pBuf-���淵�ؽ��������
       len-����pBuf�Ŀ��ó���
       ms - popen�󣬻�ȡ������֮ǰ����Ҫdelay��ʱ�䣬��λ����
���:
����: �ɹ���������ִ�еĽ��,���򷵻�XERROR
˵��: ��API���ᵼ���ļ��̳�
************************************************************************/
XPUBLIC XS32 XOS_ExeCmdByPopen_Ex(XCHAR *pCmd, XCHAR *pBuf, XU32 len, XU32 ms);



/************************************************************************
������:    XOS_GetSysRunTime
���ܣ���ȡϵͳ����ʱ��(��ȷ����)
����\
�����pSecs -   ��ǰϵͳ����ʱ�䣨�룩
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ص���ϵͳ�������������
************************************************************************/
XS32 XOS_GetSysRunTime(XU64 *pSecs);

/************************************************************************
������: XOS_GetCpuCoreNum
���ܣ���ȡcpu����
���룺
�����
���أ�����cpu����
˵����
************************************************************************/
XS32 XOS_GetCpuCoreNum(XVOID);

/************************************************************************
������: XOS_SetThreadAffinity
���ܣ������̼߳�cpu��
���룺pCpuId : cpu�ŵ�����
      usCpuNum ����Ԫ�ظ��� 
�����
���أ�XSUCC - �ɹ� XERROR  -  ʧ��
˵�����ɸ����̵߳���, �������õ�����λ�þ������ǰ������̻߳��ǵ�ǰ�߳�:
1������������߳��е��ã���ʹ���̺߳�֮�󴴽��������̰߳���Ч
2������������߳��е��ã�ֻ�Ե�ǰ�߳���Ч\
************************************************************************/
XS32 XOS_SetThreadAffinity(const XU16 *pCpuId, XS32 CpuNum);

/************************************************************************
������: XOS_SetThreadAffinity_t
���ܣ������̼߳�cpu��, ��ָ�����̵߳��߳�id
���룺thread : ��Ҫ��cpu���߳�id
      pCpuId : cpu�ŵ�����
      usCpuNum ����Ԫ�ظ��� 
�����
���أ�XSUCC - �ɹ� XERROR  -  ʧ��
˵����
************************************************************************/
XS32 XOS_SetThreadAffinity_t(pthread_t thread,const XU16 *pCpuId, XS32 CpuNum);

/************************************************************************
 ������: XOS_GetLogicPid
 ����: ��ȡ��ǰ���̵��߼����̺�
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetLogicPid(XVOID);

/************************************************************************
 ������: XOS_GetNeId
 ����: ��ȡ��ǰ���̵���Ԫid
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetNeId(XVOID);

/************************************************************************
 ������: XOS_GetWorkspaceId
 ����: ��ȡ��ǰ���̵Ĺ�����id
 ����:
 ���:
 ����: �ɹ����طǸ���, ���򷵻�XERROR
 ˵��: ֻ���ҵ����Ԫʹ��
************************************************************************/
XS32 XOS_GetWorkspaceId(XVOID);

XS32 XOS_GetNeTypeId(XVOID);
XS8* XOS_GetNeTypeStr(XVOID);
XS8* XOS_GetProcTypeStr(XVOID);

XS32 XOS_GetTsHighSlot(XVOID);

XS32 XOS_GetTsLowSlot(XVOID);

#endif

/************************************************************************
���ܣ�convert a network address to dotted decimal notation
���룺
inetAddress  -   Ҫת����in_addr
pString      -   �������淵�صĵ��ʮ����IP
�����
pString      -   ���ʮ����IP
���أ�
˵����pStringΪ����16���ַ����飬��XCHAR pString[16]
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
                �ṹ��ö������
-------------------------------------------------------------------------*/
/*�������ȼ�ö�ٶ���*/
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

#ifdef XOS_SOLARIS  /*solaris �����ȼ��ɵ͵���Ϊ1~59*/
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
    XUTIME     time;          /*��Ԫ1970.1.1���������*/
    XU16     millitm;       /*������*/
    XS16     timezone;      /*Ŀǰʱ����UTC����ʱ�䣬��λΪ����*/
}t_XOSTB;

#if 0
typedef struct _xos_timeb
{
    XU32     time;          /*��Ԫ1970.1.1���������*/
    XU16     millitm;       /*������*/
    XS16     timezone;      /*Ŀǰʱ����UTC����ʱ�䣬��λΪ����*/
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
    XU32 dwNumberOfProcessors;   /* 1.CUP����*/
    XU32 dwProcessorLoad;    /* 2.CUPռ����*/
    t_XOSTD localtime;   /* 5.����ʱ��*/
    
    XU32 dwMemoryLoad;     /* 1.�ڴ�ռ����*/
    XU32 dwTotalPhys;    /* 2.�����ڴ��ܶ�*/
    XU32 dwAvailPhys;    /* 3.�����ڴ����*/
    XU32 dwTotalPageFile;    /* 4.�������ܶ�*/
    XU32 dwAvailPageFile;   /* 5.����������*/

    XU32 dwtotalthreads;    /* 1.���ٸ�����*/
    XU32 dwthreadRun;    /* 2.���ٸ�����������(linux�п���)*/
    XU32 dwthreadssleep;    /* 3.���ٸ�����˯����(linux�п���)*/
}t_XOSSYSINFO;

#define MAX_LOCALIP_NUM    16
typedef struct _LOCALIP_LIST
{
    XU32  nIPNum;
    XU32  localIP[MAX_LOCALIP_NUM];
} t_LOCALIPLIST;


/*-------------------------------------------------------------------------
                API ����
-------------------------------------------------------------------------*/

/************************************************************************
������:    XOS_StrtoIp
���ܣ��������ַת������������Ƶ�����
���룺
pString  -   Ҫת���������ַ
inetAddress - �������淵�ص���������Ƶ�����
�����inetAddress - ��������Ƶ�����
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_StrtoIp( XCHAR *pString , XU32 *inetAddress );


/************************************************************************
������:    XOS_StrNtoIp
���ܣ��������ַת������������Ƶ����֣�����������ַ�ַ���������'\0'������
      ��ֻ��ǰlen���ַ����д���;
      ���磺�Ϸ�������"168.0.2","168.0.2."
���룺
pString  -   Ҫת���������ַ��
inetAddress - �������淵�ص���������Ƶ�����
len    -    �����ַ�ַ���pString�ĳ���
�����inetAddress - ��������Ƶ�����
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����������ϵͳ�´��ڲ��죺����"202.0.2."��Windows��Linux�±���Ϊ�ǲ�����
      ��ʽ��IP��ַ������Solaris��vxWork��ȴ�ܳɹ�ת�������ԣ�Ϊ��
      ������ʹ���ϵ����⣬�������"168.0.2.258"�ĸ�ʽ����IP��
************************************************************************/
XS32 XOS_StrNtoIp ( XCHAR *pString , XU32 *inetAddress , XU32 len);


/************************************************************************
������:    XOS_IptoStr
���ܣ�����������Ƶ�����ת���������ַ
���룺
inetAddress  -   Ҫת�����������������
pString - �������淵�صĵ��ʮ����IP
�����pString - ���ʮ����IP
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����pStringΪ����16���ַ����飬��XCHAR pString[16]
************************************************************************/
XS32 XOS_IptoStr( XU32 inetAddress , XCHAR *pString );


/************************************************************************
������:    XOS_IpNtoStr
���ܣ�����������Ƶ�����ת���������ַ��д��pString�ֻд��ǰlen���ַ�����'\0'ֹ��
���룺
inetAddress  -   Ҫת����������������֣�ע�ⲻ�ܳ�����Χ��
pString - ��������ת����IP�ַ����Ļ�����������Ϊ��д�ģ�
len    -    ������pString�ĳ���
�����pString - ���ʮ����IP
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����һ��pStringΪ����16���ַ�����ռ䣬��XCHAR pString[16];
ע��1.����ϵͳ����inet_ntoa��Ե�ʣ��������߳�ͬʱ���ô˺�����
      �ڵ�һ���ķ���ֵ��û����ʱ���ܻᱻ�ڶ����ķ���ֵ�����ǵ�
    2.VXWORKS�µ��õ�ϵͳ����Ϊ inet_ntoa_b ����Ϊinet_ntoa ��
    Each time this routine is called, 18 bytes are allocated from memory.  
    
************************************************************************/
XS32 XOS_IpNtoStr ( XU32 inetAddress , XCHAR *pString , XU32 len);


/************************************************************************
������:    XOS_StrNtoXU32
���ܣ����ַ���ת���ɳ���������ֻת��ǰlen���ַ���;
���룺
string    -    Ҫת�����ַ���
len    -    �ַ���string�ĳ���
�����value    -    ת���õ��ĳ�������
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����ת��ʱ������ǰ��Ŀո��ַ���ֱ����������ʱ�ſ�ʼת����
      ���ٴ����������ֻ��ַ���������'\0'ʱ���ء�
************************************************************************/
XS32 XOS_StrNtoXU32( XCHAR *string , XU32 *value ,XU32 len );


/************************************************************************
������:    XOS_XU32NtoStr
���ܣ����޷��ų�������ת�����ַ�����size������д��������'\0'ֹ������ֻд��size����
���룺
value    -    Ҫת�����޷�������
str    -    ��������ת������ַ���
size    -    �ַ����ĳ���
�����str    -    ת���õ����ַ���
����ֵ��
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵��: windows ����ultoa��LINUX����snprintf
************************************************************************/
XS32 XOS_XU32NtoStr(XU32 value, XCHAR *str , XU32 size );


/************************************************************************
������:    XOS_GetSysTime
���ܣ���ȡϵͳʱ��(��ȷ��ǧ��֮һ��)
���룺timex -   ��������ʱ��Ľṹ��
�����timex -   ��ǰϵͳʱ�䣨��,���뼰 Ŀǰʱ����UTC����ʱ�䣬��λΪ���ӣ�
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵�������ص��ǹ�Ԫ1970.1.1����������������
      vxWorks�·��ص��ǰ�������������
ע��vxWorks�£���������ȷ�ĳ�ʼʱ�䣬����Ի�ȡ��ǰ����ʵʱ��            
************************************************************************/
XS32 XOS_GetSysTime( t_XOSTB *timex );


#if 0
/************************************************************************
������:    XOS_GetSysInfo
���ܣ���ȡϵͳ��Ϣ
���룺N/A
�����systeminfo    -    ָ��ϵͳ��Ϣ�ṹ���ָ��
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_GetSysInfo(t_XOSSYSINFO *systeminfo);
#endif

/************************************************************************
������:    XOS_MutexCreate
���ܣ��������ĳ�ʼ��
���룺mutex    -    ������ID
�����N/A
���أ�
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_MutexCreate(t_XOSMUTEXID *mutex );


/************************************************************************
������:    XOS_MutexLock
���ܣ���û�����
���룺mutex    -    ������ID
�����N/A
���أ�
XSUCC        -    �ɹ�
XERROR    -    ʧ��
˵������������ͬһ���̵Ĳ�ͬ�̼߳�ʹ��;
      ����ͬһ�߳��ڿ��Զ��lock���������XOS_MutexUnlock�ɶԳ��֣�
************************************************************************/
XS32 XOS_MutexLock(t_XOSMUTEXID *mutex);


/************************************************************************
������:    XOS_MutexUnlock
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_MutexUnlock(t_XOSMUTEXID *mutex);


/************************************************************************
������:    XOS_MutexDelete
���ܣ�����������
���룺mutex    -    ������ID
�����N/A
���أ�
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_MutexDelete(t_XOSMUTEXID *mutex);

/************************************************************************
������:    XOS_SemCreate
����:�����ź���(����)
����:
Input:
initNum    -    �����ź����ĳ�ֵ
name      -      �ź�������
output:    semaphore   - �ź���id
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����vxWorks��֧�ֵ����ֵ������XU32�����ֵ��
      ��Windows��linux��Solarisֻ��֧��XU32���ֵ��һ��(2147483647?)
************************************************************************/
XS32 XOS_SemCreate(t_XOSSEMID *semaphore, XU32 initNum );


/************************************************************************
������:    XOS_SemGet
����:����ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_SemGet(t_XOSSEMID *semaphore);

/************************************************************************
������:    XOS_SemGetExt
����:����ź���
����:
Input:    semaphore    -    �ź�����id
          timeout   ��ʱʱ�䣬��λ ��
Return:
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����  
************************************************************************/
XS32 XOS_SemGetExt(t_XOSSEMID *semaphore, XS32 timeout);

/************************************************************************
������:    XOS_SemPut
����:�ͷ��ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_SemPut(t_XOSSEMID *semaphore);


/************************************************************************
������:    XOS_SemDelete
����:ɾ���ź���
����:
Input:    semaphore    -    �ź�����id
Return:
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_SemDelete(t_XOSSEMID *semaphore);


/************************************************************************
������:    XOS_TaskCreate
����:��������
����:
Input:
pTaskName    -    ������
iTaskPri    -    �������ȼ�
iTaskStackSize    -    ��ջ��С
fTaskFun    -    ��������
pPar    -    ����
Output:    idTask    -    �����ʶ
Return:
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_TaskCreate(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask);



/************************************************************************
������: XOS_TaskCreate_Ex
����  : ��������
Input:
        pTaskName        -    ������
        iTaskPri        -    �������ȼ�
        iTaskStackSize    -    ��ջ��С
        fTaskFun        -    ��������
        pPar            -    ����      
        pOwnPar :       -    ���������ڲ�ʹ�õģ�Ŀǰ��������cpu����Ϣ
Output:    
        idTask            -    �����ʶ
Return:
        XSUCC        -    �ɹ�
        XERROR        -    ʧ��
˵��  ����XOS_TaskCreate���˸�����pOwnPar
************************************************************************/
XS32 XOS_TaskCreate_Ex(const XCHAR *pTaskName, e_TSKPRIO iTaskPri, XS32 iTaskStackSize,
                    os_taskfunc fTaskFun, XVOID *pPar, t_XOSTASKID *idTask, XVOID *pOwnPar);




/************************************************************************
������:    XOS_TaskDel
����:ɾ������(������)
����:
Input:    idTask    -    �����ʶ
Output:    N/A
Return:
XOS_SUCC    -    �ɹ�
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_TaskDel(t_XOSTASKID idtask);


#if 0
/************************************************************************
������:    CM_Newkey
����:�½�һ����
����:
Input:     N/A    
Return:
�ɹ�    -    ָ��һ����ֵ��ָ��
ʧ��    -    XOS_ERROR
˵����
************************************************************************/
 XVOID    *CM_Newkey(XVOID);    /*��3�������ڲ����ܻ�ʹ�ã���Ϊ�գ���Ҫʱ�ټ��� mod by wentao*/


/************************************************************************
������:    CM_Setkey
����:���߳����ݺ�һ��������һ��
����:
Input:
key    -    ָ��һ����ֵ��ָ��
pointer    -    ָ��Ҫ�󶨵����ݽṹ��ָ��
Return:
�ɹ�    -    XOS_SUCC
ʧ��    -    XOS_ERROR
˵����
************************************************************************/
 XU32 CM_Setkey(XVOID *key, XCONST XVOID *pointer);


/************************************************************************
������:    CM_Getkey
����:ȡ�ü��󶨵��߳�����
����:
Input:     key    -    ָ��һ����ֵ��ָ��
Return:
�ɹ�    -    ���ؼ��󶨵�����(��0)    
ʧ��    -    XOS_ERROR
˵����
************************************************************************/
 XVOID *CM_Getkey(XVOID *key);
#endif


/************************************************************************
������:    XOS_SusPend
���ܣ���������
���룺N/A
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_SusPend( XVOID );

/************************************************************************
������:    XOS_Sleep
���ܣ�����˯��һ��ʱ��
���룺ms    -   ʱ�䳤��(million second)
�����N/A
���أ�
XSUCC    -    �ɹ�
XERROR    -    ʧ��
˵����
************************************************************************/
XS32 XOS_Sleep(XU32 ms);

/************************************************************************
������: XOS_GetLocalIP
����: ��ȡ���� IP��ַ
����: ��
���:
pLocalIPList.nIPNum; ��ȡ��IP �ĸ���, 
pLocalIPList.localIP[MAX_LOCALIP_NUM]; 32 λ IP .
eg. pLocalIPList.localIP[0] �ǵ�һ������IP��ַ.
����: ���� IP ��ַ
˵��: 
************************************************************************/
XS32 XOS_GetLocalIP( t_LOCALIPLIST* pLocalIPList);


#if 0
/************************************************************************
������:    XOS_Assert
���ܣ����ʽ�����ȷ�Բ��Բ���ʹ������ֹ
���룺exp    -    ��Ҫ�жϵı��ʽ;
�����
����ֵ����������е��߼����ʽֵΪ�ٵĻ�������ʾ�������ĸ��ļ�����һ�з����˶��Դ���;
˵����
************************************************************************/
XVOID XOS_Assert(XS32 exp );
#endif


#if 0
/************************************************************************
������:    CM_GetFilePath
���ܣ�  �õ��ļ�·��
���룺
filename - �ļ�����
buffer   - װ·�����Ļ�����
len      - װ·�����Ļ���������
�����N/A
���أ�
�ɹ� �� ʵ����Ҫ�Ļ���������
XOS_ERROR    -    ʧ��
˵����
************************************************************************/
XS32 CM_GetFilePath(XCHAR *filename, XCHAR *buffer, XU32 len);
#endif
/************************************************************************
 ������: XOS_GetPID( XVOID )
 ����:   ��ȡ��ǰƽ̨�� PID
 ����:   ��
 ���:   ��ǰƽ̨�� PID
 ����:
 ˵��: �˺���Ϊ���ã������� xosmodule.c ��
************************************************************************/
XPUBLIC XU32 XOS_GetLocalPID( XVOID );

XVOID XOS_SetLocalPID(XU32 localpid);


/************************************************************************
������: XOS_Root
���ܣ�  ƽ̨�����ں���
���룺  XVOID
�����  N/A
���أ�  XSUCC OR XERROR
˵����  ����ƽ̨����ĳ�ʼ����������ҵ�����д��ע��ṹ��������
        �Ĵ��������ȵȣ������� xosroot.c �ڣ�
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
������:XOS_Strerror
����:  �������errno��ת��Ϊ��Ӧ�Ĵ�����Ϣ�ַ���
����:  errnum  ������
���:  
����: success���ش����errmsg�ĵ�ַ��fail����"unknown errno!";
˵��: ��API֧������ƽ̨
************************************************************************/
XCHAR* XOS_StrError(XS32 errnum);

/************************************************************************
������: XOS_StackDump
���ܣ�  ����������ջ��Ϣ��¼��dump�ļ�
���룺  fid           - ���ܿ�id
�����  N/A
���أ�  XVOID 
˵���� 
************************************************************************/
XVOID XOS_StackDump(void);



#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/

