/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**  
**  Core Network Department  platform team  
**
**  filename: xostype.h
**
**  description:  ���ļ��ṩͨ�õ����Ͷ���� ��
                        �õĺ��Լ�����Ķ���. ���е�ƽ̨
                        ͷ�ļ�������ֱ�ӻ��Ӱ�����ͷ�ļ�.
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
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/                  
#include <stdio.h>

/*-------------------------------------------------------------------------
                �궨��
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
/*����ֵȫ���Ը�ֵ���壬���ݸ�ҵ���ʹ�������������*/

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
/*����ֵȫ���Ը�ֵ���壬���ݸ�ҵ���ʹ�������������*/

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
/*����ֵȫ���Ը�ֵ���壬���ݸ�ҵ���ʹ�������������*/

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
/*����ֵȫ���Ը�ֵ���壬���ݸ�ҵ���ʹ�������������*/

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

/*64λ���ļ���С���Գ���4G*/
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

/* �������*/
#ifndef XOS_DECLARE_HANDLE
#define XOS_DECLARE_HANDLE(name)    typedef struct { int unused; } name##__ ; \
                typedef const name##__ * name; \
                typedef name*  LP##name
#endif

/*-------------------------------------------------------------------------
                �ṹ��ö������
-------------------------------------------------------------------------*/
/*ip ��ַ����*/
typedef struct
{
    XU32 ip;
    XU16 port;
}t_IPADDR;

/*sctp ��ַ�ṹt_SCTPIPADDR֧�ֵ�IP����*/
#define SCTP_ADDR_NUM       8
/*SCTP ip ��ַ����*/
typedef struct
{
    XU32 ip[SCTP_ADDR_NUM];
    XU16 port;
    XU16 ipNum;
}t_SCTPIPADDR;

/*��Ϣ���ȼ�*/
typedef enum
{
    eMinPrio        = 0,
    eChuckMsgPrio   = 1,   /*�ɶ�������Ϣ���ȼ���*/
    eNormalMsgPrio  = 2,   /* һ����Ϣ���ȼ���*/
    eFastMsgPrio    = 3,   /*������Ϣ���ȼ���*/
    eAdnMsgPrio     = 4,   /*������Ϣ���ȼ���*/
    eStandbyPrio    = 5,   /*������Ϣ���ȼ���*/
    eTimePrio       = 6,   /*��ʱ����Ϣ���ȼ���*/
    eHAPrio         = 7,   /*HA��Ϣ���ȼ���*/
    eMAXPrio
}e_MSGPRIO;

/*��Ϣͷ��صĶ���*/
typedef struct _XOSMSGHEAD
{
    XU32  PID;  /*��������*/
    XU32  FID;  /*���ܿ��*/
    XU32  FsmId;/* �ڲ����Ӻ�*/
}t_XOSUSERID;

typedef struct  _XOSCOMMGHEAD
{
    t_XOSUSERID datasrc;  /*��Ϣ��Դ,��FIDȫ��Ψһ����дFID����*/
    t_XOSUSERID datadest; /*��ϢĿ��*/
    XU16       msgID;           /*��Ϣ����*/
    XU16       subID;           /*��Ϣ������*/  
    e_MSGPRIO  prio;             /*��Ϣ���ȼ�*/   
    XU32    traceID;           /*TraceID��Ա�ֶ�*/
    XU32    logID;          /*LogID��Ա�ֶ�*/
    XU32       length;          /*��Ϣ����*/   
    XVOID      *message;        /*��Ϣָ��*/ 
} t_XOSCOMMHEAD;

typedef enum
{
   PL_MIN = 0,
   PL_DBG,        /*�ܵ͵Ĵ�ӡ����(�����ʹ�ӡ)*/
   PL_INFO,       /*������Ϣ��ӡ����(�����ʹ�ӡ)*/
   PL_WARN,       /*������Ϣ��ӡ����(���д�ӡ)*/
   PL_ERR,        /*һ���Դ����ӡ����(�����ʹ�ӡ)*/
   PL_EXP,        /*�쳣��ӡ����(�����ʹ�ӡ)*/
   PL_LOG,        /*�����ӡ����־�ļ��е���Ϣ*/
   PL_MAX
}e_PRINTLEVEL;
/*-------------------------------------------------------------------------
API ����
-------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xos.h*/

