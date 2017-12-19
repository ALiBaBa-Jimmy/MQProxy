/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: xoslog.h
**
**  description:  主要定义了通用数据结构和模块接口函数
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
**    lxn            2006.8.17         modify    按照第二版的设计重新code
**    lxn            2006.11.20        modify    完成三期设计代码实现
**    lixn           2006.11.22        modify    代码检视
**************************************************************/

/*******************************************\
                    log模块的头文件

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


#define  MAX_FILE_LEN              (252)     /*-定义日志文件每行的最大输入字符数*/
#define  LOGDIR                    "log/"  /*日志文件存储的文件夹名*/

#define  LOG_ONE_MEGA    (1)
#define  LOG_BUF_SIZE    (1024*1024*2)     /*2M*/
#define  MIN_LOGFILESIZE (1024)          /*配置文件中定义的日志文件大小最小取值1M,默认日志文件大小*/
#define  MAX_LOGFILESIZE (1024*20)         /*配置文件中定义的日志文件大小最大取值20M*/
#define  MIN_CAP         (2*MAX_LOGFILESIZE) /*最小硬盘可写容量*/
//modified for xos log optimization zenghaixin 20100817
#define  MAX_LOGS_SIZE   (1024*1024*500)   /*500M，log空间限制*/ 
#define  MAX_LOGS        (512)             /*log个数限制*/ 

//end modified for xos log optimization zenghaixin 20100817


#if ( defined (XOS_LINUX) || defined (XOS_SOLARIS) )
#define LOG_MODE      S_IRWXU   /*LINUX下创建文件的权限规定,用户可读写执行*/
#endif

/*-----------------------结构定义----*/

#define FID_CHECK_NUM (50)  /*流量检测峰值*/

typedef struct logUdpSocket
{
    t_XINETFD       SockId;        /*数据连接信息索引*/
    t_IPADDR        ServerAddr;  //需要发送消息的IP/port
    t_IPADDR        LocalAddr;  
}logTcpSocket;

typedef struct
{
   XU32 fid_msgmax;          /*设置最大流量*/
   XU32 fid_msgcount;        /*FID在当前2秒内已经处理的消息条数*/
   XU32 fid_msgfilter;       /*FID在当前2秒内被过滤的消息条数(大于流量就过滤)*/
   XU32 fid_msgchktime;      /*时刻记录，单位秒*/
   XU32 fid_msgpeakcount;    /*峰值率*/
}t_MSGFIDCHK;


/*-记录系统事件格式*/
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


/*定义缓冲区结构需要打印输出*/
typedef struct _LOGBUFFER
{
     XCHAR *pLogBufferHead;  /*缓冲区的头*/
     XCHAR *pLogBufferTail;  /*缓冲区尾*/
     XCHAR *pLogBufferLast;  /*缓冲区中实际写入数据的最尾部指针*/
     XU32  logBufferLen;     /*缓冲区总长度*/
     XU32  logBufferUsedLen; /*缓冲区已记录的数据长度*/
     XS32 buf_state;         /*该buffer的状态位。1写状态;0空闲状态，-1传输状态,-2传输等待状态*/
}t_LOGBUFFER;

/*报文起始标志*/
#ifndef LOGDATA_BEGIN_FLAG
#define LOGDATA_BEGIN_FLAG 0x7ea5
#endif

/*报文结束标志*/
#ifndef LOGDATA_END_FLAG
#define LOGDATA_END_FLAG 0x7e0d
#endif 

#ifndef LOG_ITEM_MAX
#define LOG_ITEM_MAX (1000)  /*每条日志信息最大长度,2008-03调整和MAX_TRACE_INFO_LEN宽度一样*/
#endif

/*log报文类型*/
typedef enum e_LogType {
    e_SHAKEREQ = 0,               /*心跳请求*/
    e_SHAKERSP,                   /*心跳响应*/
    e_LOGREQ                      /*数据报文*/
}e_LogType;

#pragma pack(1)

/*报文头部*/
typedef struct t_LogDataHead
{
    XU16   _nBeginFlag;              /*起始标识*/
    XU16   _nSize;                   /*结构总长度*/
    XU8    _nType;                   /*日志类型 0为心跳请求报文，1为心跳响应报文，2为上传日志报文。无符号单字节整型*/
    XU32   _nSn;                     /*序列号*/
}t_LogDataHead;

typedef struct t_LogDataTail
{
   XU16   _nEndFlag;                /*结束标识*/    
}t_LogDataTail;

/*握手请求报文*/
typedef struct t_LogShakeReq
{
    t_LogDataHead _tHead;            /*报文头*/
    XU16   _nEndFlag;                /*结束标识*/    
}t_LogShakeReq;

/*握手响应报文*/
typedef struct t_LogShakeRsp
{
    t_LogDataHead _tHead;            /*报文头*/
    XU16   _nEndFlag;                /*结束标识*/    
}t_LogShakeRsp;

/*日志报文结构*/
typedef struct t_LogData
{
    t_LogDataHead _tHead;            /*报文头*/
    XCHAR  _bPlayMsg[LOG_ITEM_MAX];   /*最大日志内容*/
    XU16   _nEndFlag;                /*结束标识*/
 }t_LogData;

/*模块间消息结构*/
typedef struct t_LogLocal
{
    XU32   _nLogId;
    e_PRINTLEVEL _level;
    XU16 _nLen;
    XCHAR *_pszMsg;
}t_LogLocal;

#pragma pack()


/*
 *日志模块运行中的中间参数
 */
typedef struct t_LogStatus {
    XCHAR _szSystemUptime[32];               /*每次平台新启动的日期时间*/
    XCHAR _szCurrLogname[XOS_MAX_PATHLEN+1];   /*当前正在写的日志文件*/
    FILE  *_pLogFileHandle;                  /*当前打开的本地日志文件*/
    XU32  _uSeqNum ;                         /*当前正在写的日志文件在此次平台启动后建立的日志文件中的序号*/
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

XS32 Log_CheckFileFull(void);            /*检查文件是否已经写满*/

XS32 Log_CliCommandInit(int cmdMode);

XS8  Log_InitLocalLogFile(void);

XS32 Log_GetSysDiskCap(XU64 * cap);                 /*获取当前系统磁盘的可用空间*/

XS32 Log_FormatWriteLog(XCHAR *str, FILE * tofile);  /*将信息串写入日志文件*/

/*根据flag得到当前的日志文件名称字符串*/
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


