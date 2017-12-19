/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xostrace.h
**
**  description:
**
**  author: guolili
**
**  date:   2006.3.9
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   guolili         2006.3.9             create
**   lxn	            2006.7.27           modify
**************************************************************/
#ifndef _XOS_TRACE_H_
#define _XOS_TRACE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/
#include <string.h>

#include "xosos.h"
#include "xostype.h"
#include "clishell.h"

/*------------------------------------------------------------------------
                  ����ͷ�ļ�
-------------------------------------------------------------------------*/

/*-------------------------------�궨��------------------------------------*/
#define MAX_XU32INT                 0xffffffff /*����xu32���͵����ֵ*/
#define SWITCH_ON                   1 /*���ƿ���:��*/
#define SWITCH_OFF                  0 /*���ƿ���:��*/

#define MSGID_PRINTO_MCB      1

#ifndef MAX_FILE_NAME_LEN
#define MAX_FILE_NAME_LEN      128/*��ӡ����ļ���2007/11/08 adjust from 50 to 64*/
#endif
#define MAX_MSGHEAD_LEN         100 /*Ҫ��ӡ����Ϣͷ��ʽ����󳤶�*/
#define MAX_TRACE_INFO_LEN      4160/*����trace�����Ϣ�������,2007/11/08 adjust from 512 to 1024*/
                                     /*2008-08 adjust from 1024 t0 512*/
                                    /*2011-07 adjust from 512 to 800 for ipc xos_trace string len*/

/*������Ϣ�����������*/
/*FID���Ϸ�ʱ����Ϣ����Ĺ��˼���*/
#define XOS_INVALID_FID_TRACE_LEV    PL_ERR

/*FIDû��ע��ʱ����Ϣ������˼���*/
#define XOS_UNREG_FID_TRACE_LEV      PL_ERR

/*FID����ʱ��Ĭ��ģ�����Ϣ������˼���*/
#define XOS_FID_INIT_TRACE_LEV       PL_ERR

#define XOS_PRINT                    XOS_Trace /*������ӡ�汾֮��ļ���*/
#define XOS_PrintInit                Trace_Init /*ģ���ʼ��ʱ���ְ汾�ļ���*/

/*��ӡʱ����ĸ�ʽ: XOS_PRINT(MD(ulFid, ulLevel), format...);�еĺ궨��*/
#ifdef XOS_LINUX
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__), (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#else
#define MD( ulFid, ulLevel) (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL, (XU32 )(ulFid), (e_PRINTLEVEL)(ulLevel)
#endif

#ifdef XOS_LINUX
#define FILI (XCHAR*)(__FILE__), (XU32)(__LINE__), (XCHAR*)(__FUNCTION__)
#else
#define FILI (XCHAR*)(__FILE__), (XU32)(__LINE__), NULL
#endif
/*-------------------------------------------------------------------------

                �ṹ��ö������
-------------------------------------------------------------------------*/



typedef enum    /*��Ϣ���Ŀ���ļ�ö�ٽṹ*/
{
   TOCOSOLE = 0,     /*ֻ��ӡ���ն�*/
   TOLOGFILE = 1,    /*ֻ��ӡ���ļ�*/
   TOFILEANDCOS = 2, /*��ӡ���ն˺��ļ�*/
   TOTELNET = 3,     /*��ӡ��telnet����չ*/
   TOSERVER = 4,     /*��ӡ��Զ�˷���������չ*/
   MAX_DEV
}e_TRACEPRINTDES;

/*fid ��ص�trace��Ϣ���ƿ��ؽṹ��*/
typedef struct _TRACE_FID
{
 XBOOL isNeedFileName;     /*�Ƿ���Ҫ��ӡ�ļ�����XTRUE��ʾ��Ҫ��ӡ��XFALSE��ʾ����Ҫ��ӡ*/
 XBOOL isNeedLine;         /*�Ƿ���Ҫ��ӡ�кţ�XTRUE��ʾ��Ҫ��ӡ��XFALSE��ʾ����Ҫ��ӡ*/
 XBOOL isNeedTime;         /* �Ƿ���Ҫ��ӡʱ�䣬 XTRUE��ʾ��Ҫ��ӡ��XFALSE��ʾ����Ҫ��ӡ*/
 XBOOL isPrintInTraceTsk;  /*�Ƿ���trace������ռ��д�ӡ��XTRUE��ʾ�ڡ�XFALSE��ʾ���û��Լ��������д�ӡ*/
 e_PRINTLEVEL traceLevel;  /*ģ��Ĵ�ӡ����*/
 e_PRINTLEVEL sessionLevel[kCLI_MAX_CLI_TASK];  /*ģ���telnet�ն˴�ӡ����*/
 e_TRACEPRINTDES traceOutputMode;
 t_XOSTT prevTime;                    /*��һ�η���ʱ��*/
 t_XOSMUTEXID xosFidTraceMutex;       /*�ٽ���*/
}t_FIDTRACEINFO;

/*-��¼ϵͳ�¼���ʽ*/
typedef struct _TIME
{
    XU32    dt_sec;         /* seconds */
    XU32    dt_min;         /* minutes */
    XU32    dt_hour;        /* hours */
    XU32    dt_mday;        /* day of the month */
    XU32    dt_mon;         /* month */
    XU32    dt_year;        /* year */
    XU32    dt_wday;        /* day of the week */
    XU32    dt_yday;        /* day in the year */
    XU32    dt_isdst;       /* daylight saving time */
}t_trc_systime;

/*------------------------�ṹ����------------------------------------*/


/*---------------------------------------------------------------------
��������  XOS_CpsTrace
���ܣ�   ���ƽ̨�ڲ���trace��ӡ��Ϣ�������ӡ���ն˲��Ҽ�¼����־�ļ���
���룺

        ulFid           - ���룬���ܿ�ID
        ulLevel         - ���룬��ӡ����
        ucFormat        - ���룬��ӡ��ʽ���ַ���
        ...             - ���룬��ӡ����

�����
����ֵ:  XSUCC  -	�ɹ�		  XERROR -	ʧ��
˵����
------------------------------------------------------------------------*/
XPUBLIC XVOID XOS_CpsTrace( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... );

/*---------------------------------------------------------------------
��������  XOS_CpsPrintf
��  �ܣ�  ���ƽ̨�ڲ���trace��ӡ��Ϣ�������ӡ���ն˲��Ҽ�¼����־�ļ���
��  �룺  ulFid            - ���룬���ܿ�ID
          ulLevel          - ���룬��ӡ����
          etraceOutputMode - ���룬������ն˻��ļ� 
          ucFormat         - ���룬��ӡ��ʽ���ַ���
          ap               - ���룬��ε�ջ��ʼ��ַ

��  ����
����ֵ:  XSUCC  -   �ɹ��� XERROR - ʧ��
˵  ���� �������ĳ��ȴ���MAX_TRACE_INFO_LEN�����ʧ�ܣ��Ҳ�������ʾ
------------------------------------------------------------------------*/
XVOID XOS_CpsPrintf(XU32 ulFid, e_PRINTLEVEL eLevel, e_TRACEPRINTDES etraceOutputMode, const XCHAR *cFormat, va_list ap);

XVOID XOS_TraceTa( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap );
XVOID XOS_TraceInfo( const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName,XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat,va_list ap ,XCHAR *buf);



#define XOS_Trace XOS_CpsTrace
#define XOS_Printf XOS_CpsPrintf

/*---------------------------------------------------------------------
��������  XOS_Trace
���ܣ�    �°���Ϣ��ӡ����
���룺
         FileName       - ���룬�ļ���(��__FILE__)
        ulLineNum       - ���룬�к�   (��__LINE__)
        ulFid           - ���룬���ܿ�ID
        ulLevel         - ���룬��ӡ����
        ucFormat        - ���룬��ӡ��ʽ���ַ���
        ...             - ���룬��ӡ����

�����
����ֵ:  XSUCC  -	�ɹ�		  XERROR -	ʧ��
˵����
------------------------------------------------------------------------*/
#if 0
XPUBLIC XVOID XOS_Trace( const XCHAR* FileName, XU32 ulLineNum, XU32 ulFid, e_PRINTLEVEL eLevel, const XCHAR *cFormat, ... );
#endif

/******************************************************************
������: Trace_Init
����:ϵͳ����ʱ�Դ�ӡģ��ĳ�ʼ������

*******************************************************************/
XEXTERN XS8  Trace_Init(XVOID * t, XVOID *V);

/*---------------------------------------------------------------------
��������  XOS_Sprintf
���ܣ�     ��ʽ���ַ�������.��sprintf������ͬ
                    �˺����У����ȶ�cFormat�����и�ʽ����������ʽ������ַ�������������
                    500���ַ������ڡ���ʽ�������cFormat����ʽ������ַ�������ʵ��Ҫд��Ļ�������С
                    �����������Ϣ�����򽫸�ʽ������ַ���д��ʵ�ʻ�����
���룺
        Buffer               - ���룬��Ÿ�ʽ������ַ����Ļ�����.
        charnum              - ���룬ʵ��Ҫд�뻺�������ַ�������.
        cFormat              - ���룬��ʽ�����ַ���
		...                  - ����, �������������ǰ��ĸ�ʽ���ַ���ƥ��
�����
����ֵ: �ɹ�ʵ��д����ֽ���
                  ʧ�ܷ��� XERROR
˵����
------------------------------------------------------------------------*/
XEXTERN XS32 XOS_Sprintf( XCHAR* Buffer,XU32 charnum, const XCHAR *cFormat, ... );

/*---------------------------------------------------------------------
��������  XOS_getPLName
���ܣ�    ��ȡ��ӡ���������
���룺
�����
����ֵ:
˵����
------------------------------------------------------------------------*/
XCHAR* XOS_getPLName(e_PRINTLEVEL printLevel);

/*------------------------------------------------------------------
��������  Trace_msgProc
���ܣ�   ��Ϣ������
���룺
�����
����ֵ:
˵����
------------------------------------------------------------------------*/
XPUBLIC XS8 Trace_msgProc(XVOID* pMsP, XVOID*sb );
/******************************************************
��������  Trace_abFileName
���ܣ�   �Ӿ���·���н����ļ�������
���룺  FileName -����·��
                   fnstr--���յ��ļ���
                   Max_FileNameLen--�ļ�������󳤶�
�����
����ֵ:   �ɹ�����xsucc;ʧ�ܷ���xerror;
˵����

*******************************************************/
XPUBLIC XS32 Trace_abFileName(XCONST XCHAR *FileName,XCHAR *fnstr,XU32 Max_FileNameLen);

/*--------------------------------------------------------------------
��������  XOS_GetFidTraceFlag
���ܣ�    ��ȡFID��ǰtrace��Ϣ�����־λ
���룺
        ulFid -���ܿ�ID
      ulFidTraceInfo -�����¼trace�ĸ�����־λ�ṹ

�����
����ֵ:  �ɹ�����XSUCC��ʧ�ܷ���XERROR
˵����
------------------------------------------------------------------------*/
XPUBLIC XS32 XOS_GetFidTraceFlag(XU32 ulFid, t_FIDTRACEINFO *ulFidTraceInfo);

/*--------------------------------------------------------------------
��������  XOS_SetFidTraceFlag
���ܣ�    ����FID��Ӧtrace��Ϣ�����־λ
���룺
      ulFid -���ܿ�ID
      FilenameFlag - �����Ƿ��ӡ�ļ�����־��TRUE-��ӡ��FALSE-����ӡ��
      LinenumFlag -�����Ƿ��ӡ�кű�־��TRUE-��ӡ��FALSE-����ӡ��
      TimeFlag -�����Ƿ��ӡʱ���־��TRUE-��ӡ��FALSE-����ӡ��
      TransToTaskFlag -�����Ƿ��䵽��Ϣ���б�־��TRUE-����FALSE-������
      OutputMode -�������Ŀ���豸��־����e_TRACEPRINTDES �ṹ��
      OutputLevel -���ø�FID����Ϣ��ӡ���˼��������Ϣ������ڸü���ʱ�����������e_PRINTLEVEL�ṹ

�����
����ֵ:  �ɹ�����XSUCC��ʧ�ܷ���XERROR
˵����
------------------------------------------------------------------------*/
XPUBLIC XS32 XOS_SetFidTraceFlag(XU32 ulFid,XBOOL FilenameFlag,XBOOL LinenumFlag,XBOOL TimeFlag,
              XBOOL TransToTaskFlag, e_TRACEPRINTDES  OutputMode, e_PRINTLEVEL OutputLevel);
XPUBLIC XS8 XOS_TraceClose();

/************************************************************************
 ������: XOS_PrinToMcb(  )
 ����:   ����ʽ���������,�����Ϣ�����ض���
 ����:   pFmt ��ʽ���ַ���
 ���:   ��
 ����:   ��
 ˵��:   ��
************************************************************************/
XS32 XOS_PrinToMcb( XCHAR* pFmt, ... );
XS32 XOS_PrintToMcbStr(const XCHAR* buff);

XS32 XOS_TraceSetFidLevel (XU32  ulFid, e_PRINTLEVEL ulLevel);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*xostrace.h*/


