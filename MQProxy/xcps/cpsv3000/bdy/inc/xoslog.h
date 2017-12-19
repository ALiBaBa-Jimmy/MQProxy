/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xoslog.h
**
**  description:  ��Ҫ������ͨ�����ݽṹ��ģ��ӿں���
**
**  author:lixiangni
**
**  date:   2006.8.3
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
      lxn            2006.7.27         create
**    lxn            2006.8.17         modify    ���յڶ�����������code
**    lxn            2006.11.20        modify    ���������ƴ���ʵ��
**    lixn           2006.11.22        modify    �������
**************************************************************/

/*******************************************\
                    logģ���ͷ�ļ�

\********************************************/

#ifndef _XOS_LOG_H_
#define _XOS_LOG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "xosshell.h"
#include "xosxml.h"
#include "xmlparser.h"
#include "xwriter.h"
#include "xosinet.h"
#include "xosntl.h"


#define  MAX_FILE_LEN              (252)     /*-������־�ļ�ÿ�е���������ַ���*/
#define  LOGDIR                    "log/"  /*��־�ļ��洢���ļ�����*/

#define  LOG_ONE_MEGA    (1)
#define  LOG_BUF_SIZE    (1024*1024*2)     /*2M*/
#define  MIN_LOGFILESIZE (1024)          /*�����ļ��ж������־�ļ���С��Сȡֵ1M,Ĭ����־�ļ���С*/
#define  MAX_LOGFILESIZE (1024*20)         /*�����ļ��ж������־�ļ���С���ȡֵ20M*/
#define  MIN_CAP         (2*MAX_LOGFILESIZE) /*��СӲ�̿�д����*/
//modified for xos log optimization zenghaixin 20100817
#define  MAX_LOGS_SIZE   (1024*1024*500)   /*500M��log�ռ�����*/ 
#define  MAX_LOGS        (512)             /*log��������*/ 

//end modified for xos log optimization zenghaixin 20100817


#if ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )
#define LOG_MODE      S_IRWXU   /*LINUX�´����ļ���Ȩ�޹涨,�û��ɶ�дִ��*/
#endif

/*-----------------------�ṹ����----*/

#define FID_CHECK_NUM (50)  /*��������ֵ*/

typedef struct logUdpSocket
{
    t_XINETFD       SockId;        /*����������Ϣ����*/
    t_IPADDR        ServerAddr;  //��Ҫ������Ϣ��IP/port
    t_IPADDR        LocalAddr;  
}logTcpSocket;

typedef struct
{
   XU32 fid_msgmax;          /*�����������*/
   XU32 fid_msgcount;        /*FID�ڵ�ǰ2�����Ѿ��������Ϣ����*/
   XU32 fid_msgfilter;       /*FID�ڵ�ǰ2���ڱ����˵���Ϣ����(���������͹���)*/
   XU32 fid_msgchktime;      /*ʱ�̼�¼����λ��*/
   XU32 fid_msgpeakcount;    /*��ֵ��*/
}t_MSGFIDCHK;


/*-��¼ϵͳ�¼���ʽ*/
typedef struct _logTIME
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

}t_LogInitTIME;


/*���建�����ṹ��Ҫ��ӡ���*/
typedef struct _LOGBUFFER
{
     XCHAR *pLogBufferHead;  /*��������ͷ*/
     XCHAR *pLogBufferTail;  /*������β*/
     XCHAR *pLogBufferLast;  /*��������ʵ��д�����ݵ���β��ָ��*/
     XU32  logBufferLen;     /*�������ܳ���*/
     XU32  logBufferUsedLen; /*�������Ѽ�¼�����ݳ���*/
     XS32 buf_state;         /*��buffer��״̬λ��1д״̬;0����״̬��-1����״̬,-2����ȴ�״̬*/
}t_LOGBUFFER;

/*������ʼ��־*/
#ifndef LOGDATA_BEGIN_FLAG
#define LOGDATA_BEGIN_FLAG 0x7ea5
#endif

/*���Ľ�����־*/
#ifndef LOGDATA_END_FLAG
#define LOGDATA_END_FLAG 0x7e0d
#endif 

#ifndef LOG_ITEM_MAX
#define LOG_ITEM_MAX (1000)  /*ÿ����־��Ϣ��󳤶�,2008-03������MAX_TRACE_INFO_LEN���һ��*/
#endif

/*log��������*/
typedef enum e_LogType {
    e_SHAKEREQ = 0,               /*��������*/
    e_SHAKERSP,                   /*������Ӧ*/
    e_LOGREQ                      /*���ݱ���*/
}e_LogType;

#pragma pack(1)

/*����ͷ��*/
typedef struct t_LogDataHead
{
    XU16   _nBeginFlag;              /*��ʼ��ʶ*/
    XU16   _nSize;                   /*�ṹ�ܳ���*/
    XU8    _nType;                   /*��־���� 0Ϊ���������ģ�1Ϊ������Ӧ���ģ�2Ϊ�ϴ���־���ġ��޷��ŵ��ֽ�����*/
    XU32   _nSn;                     /*���к�*/
}t_LogDataHead;

typedef struct t_LogDataTail
{
   XU16   _nEndFlag;                /*������ʶ*/    
}t_LogDataTail;

/*����������*/
typedef struct t_LogShakeReq
{
    t_LogDataHead _tHead;            /*����ͷ*/
    XU16   _nEndFlag;                /*������ʶ*/    
}t_LogShakeReq;

/*������Ӧ����*/
typedef struct t_LogShakeRsp
{
    t_LogDataHead _tHead;            /*����ͷ*/
    XU16   _nEndFlag;                /*������ʶ*/    
}t_LogShakeRsp;

/*��־���Ľṹ*/
typedef struct t_LogData
{
    t_LogDataHead _tHead;            /*����ͷ*/
    XCHAR  _bPlayMsg[LOG_ITEM_MAX];   /*�����־����*/
    XU16   _nEndFlag;                /*������ʶ*/
 }t_LogData;

/*ģ�����Ϣ�ṹ*/
typedef struct t_LogLocal
{
    XU32   _nLogId;
    e_PRINTLEVEL _level;
    XU16 _nLen;
    XCHAR *_pszMsg;
}t_LogLocal;

#pragma pack()


/*
 *��־ģ�������е��м����
 */
typedef struct t_LogStatus {
    XCHAR _szSystemUptime[32];               /*ÿ��ƽ̨������������ʱ��*/
    XCHAR _szCurrLogname[XOS_MAX_PATHLEN+1];   /*��ǰ����д����־�ļ�*/
    FILE  *_pLogFileHandle;                  /*��ǰ�򿪵ı�����־�ļ�*/
    XU32  _uSeqNum ;                         /*��ǰ����д����־�ļ��ڴ˴�ƽ̨������������־�ļ��е����*/
    XU32  _uCurSize;                         /**/
}t_LogStatus;



XSFILESIZE Log_GetFileSize(XCHAR *file);

XPUBLIC XS32 LOG_TskStackInfo(t_XOSTASKID *tskID,XCHAR *pReason);

XEXTERN XS32 TRC_formateloctime(XCHAR *date);

XVOID Log_CmdCloseRemoteLog(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdOpenRemoteLog(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);
XVOID Log_SetReadSet(void);

XVOID Log_SetWriteSet(void);

XVOID Log_SetRWSet(void);

XVOID Log_ClearReadSet(void);

XVOID Log_ClearWriteSet(void);

XVOID Log_ClearRWSet(void);

XVOID Log_TcpSelectWProc(t_FDSET *pReadSet, t_FDSET *pWriteSet);

XVOID Log_TcpSelectRProc(t_FDSET *pReadSet, t_FDSET *pWriteSet);

XVOID  Log_TcpRecv(XVOID* taskPara);

XVOID Log_CmdSetFidCheck(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdShowFidCheck(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdRestartLink(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdShowInfo(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdSetLogLevel(CLI_ENV *pCliEnv, XS32 siArgc,XCHAR **ppArgv);

XVOID Log_CmdShowFidLevel(CLI_ENV *pCliEnv, XS32 siArgc, XCHAR **ppArgv);

XBOOL Log_WriteConfigToXml(XCHAR *filename, XU8 enable, XU16 localPort, XU32 remoteAddr, XU16 remotePort, XU16 flowCtrol);

XS8  Log_SendShakeReqMsg(void);

XS8  Log_SendToLog(XU32 logId, e_PRINTLEVEL level, XCHAR* logmsg);

XS32 Log_SendDataToRemote(XU32 logid,e_PRINTLEVEL infotype, XCONST XCHAR *msg_out, XU16 msgLen);

XS32 Log_SendShakeReqToRemote(void);

XS32 Log_SendShakeRspToRemote(void);

XS8  Log_DealTcpStream(XU8* ipData, XU16 len);

XU32 Log_GetRecordSn();

XS8  Log_LinkStopAndInit();

XS8  Log_DealTcpStreamProc(XU8* pData, XU16 len);

XS8  Log_RecvShakeReqProc(XU8* pData, XU16 len);

XS8  Log_RecvShakeRspProc(XU8* pData, XU16 len);

XS8  Log_SendShakeRspMsg(void);

XS32 Log_CheckFileFull(void);            /*����ļ��Ƿ��Ѿ�д��*/

XS32 Log_CliCommandInit(int cmdMode);

XS8  Log_InitLocalLogFile(void);

XS32 Log_GetSysDiskCap(XU64 * cap);                 /*��ȡ��ǰϵͳ���̵Ŀ��ÿռ�*/

XS32 Log_FormatWriteLog(XCHAR *str, FILE * tofile);  /*����Ϣ��д����־�ļ�*/

/*����flag�õ���ǰ����־�ļ������ַ���*/
XS32 Log_CreateRunFileName(XCHAR *filename, XBOOL flag);

XS32 Log_GetLoctime(XCHAR *datet);


XS8  Log_Init ( XVOID*p1, XVOID*p2);

XS32 Log_Write ( const t_FIDCB *pFidCb, XU32 logid, e_PRINTLEVEL infotype, XCHAR *msg_out);
XS32 Log_SessionWrite ( const t_FIDCB *pFidCb, XU32 logid, e_PRINTLEVEL infotype, e_PRINTLEVEL level,XCHAR *msg_out);

XS8  Log_NoticeProc(XVOID *t, XVOID *v);

XS8  Log_TimerProc( t_BACKPARA* pBackPara);

XS8  Log_MsgProc(XVOID* pMsP, XVOID*sb );

XS32 Log_CheckFidFlow(XU32 chk_fid);

XS8  Log_StartShakeTimer(void);

XS8  Log_StopShakeTimer(void);

XS32 Log_TraceMsgPro(t_XOSCOMMHEAD* pMsg);

XS32 Log_TraceTaskProc(XS32 logId, e_PRINTLEVEL level, XCHAR* logmsg, XU16 msgLen);

XS8  XOS_LogSetLinkPara(XU8 enable, XU16 localPort, XU32 remoteAddr, XU16 remotePort, XU16 flowCtrol);

XS8  XOS_LogGetLinkPara(XU8 *pEnable, XU16 *pLocalPort, XU32 *pRemoteAddr, XU16 *pRemotePort, XU16 *pFlowCtrol);

XS8  XOS_LogGetLinkStatus(void);

XS8  XOS_LsCheckLogDir(XCHAR *pszDir);

XS32 Log_StartTcpReconnTimer(void);

XS32 Log_StopTcpReconnTimer(void);

XS8  Log_TcpLinkStart(void);

XS32 Log_CloseTcp(void);

XS32 Log_TcpReConnect(void);

XS32 Log_SendTcpData(XCONST XCHAR *pData, XS32 dataLen);

XS8  Log_StopLink(void);

XS8  XOS_LogSetFidLevel(XU32 fid, e_PRINTLEVEL logLevel);


#ifdef __cplusplus
}
#endif

#endif /*xoslog.h*/


