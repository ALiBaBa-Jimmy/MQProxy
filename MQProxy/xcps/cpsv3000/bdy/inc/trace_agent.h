/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename: trace_agent.h
**
**  description: this agent connect to trace server,receives filter message and sends
**              trace message to trace server.
**
**  author: liukai
**
**  date:   2014.8.12
**
**
**************************************************************/
#ifndef _TRACE_AGENT_H_
#define _TRACE_AGENT_H_
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef XOS_TRACE_AGENT

/*-------------------------------------------------------------
                  包含头文件
--------------------------------------------------------------*/
#include "xostype.h"
#include "xostl.h"
#include "xosinet.h"
#include "xosmodule.h"
#include "xosarray.h"
#include "xoshash.h"
#include "xosxml.h"
#include "xmlparser.h"
#include "xosenc.h"
#define FID_EMME    1000
/*-------------------------------------------------------------
                  宏定义
--------------------------------------------------------------*/
#define TA_CFG_REQ  0x0201    /*TA向OAM请求t_taCfg信息的消息ID*/
#define TA_CFG_RSP  0x0202    /*OAM向TA响应t_taCfg信息的消息ID*/

#define MME_IMSI_REQ    0x0301  /*TA向eMME发送的请求IMSI的消息*/
#define MME_IMSI_RSP    0x0302  /*eMME向TA发送的IMSI的响应消息*/
#define MME_TEL_REQ     0x0303  /*TA向eMME发送的请求电话号码的消息*/
#define MME_TEL_RSP     0x0304  /*eMME向TA发送的电话号码的响应消息*/
#define MME_SAVE_IDENTIFY       0x0305      /*NAS需要缓存与S1AP直接的鉴权消息*/
#define MME_NOT_SAVE_IDENTIFY   0x0306      /*NAS停止缓存与S1AP直接的鉴权消息*/
#define TA_DEL_REQ     0x0307  /*TA向网元模块发送的删除TraceID消息*/

#define MAX_TEL_LEN (32)
#define MAX_IMSI_LEN (15)
#define MAX_MD5_LEN (32)

#define TA_USER_NUM     2       /*感染时每个类型的用户标识支持的最大数量*/
#define TA_MAX_MSG_LEN  4096    /*TA发送给TS的最大trace数据长度*/
/*-------------------------------------------------------------------------
                     模块外部结构和枚举定义
-------------------------------------------------------------------------*/
/*TS部署模式*/
typedef enum
{
    e_SingleMode    = 0x0000,
    e_HaMode        = 0x0001
}e_TsMode;

/*用户标识类型定义*/
typedef enum
{
    e_TA_TEL = 1,  /*电话号码*/
    e_TA_IMSI      /*IMSI*/
}e_IdType;

/*接口消息方向*/
typedef enum
{
    e_TA_SEND = 0x01,     /*本端发送*/
    e_TA_RECV = 0x02         /*本端接收*/
}e_TaDirection;

/*接口过滤时的源目的地址参数*/
typedef struct
{
    t_IPADDR src;
    t_IPADDR dst;
}t_TaAddress;

/*业务感染接口参数，最多同时支持两个电话号码和两个IMSI*/
typedef struct
{
    XU16 idNum;
    XU16 imsiNum;
    XU8 id[TA_USER_NUM][MAX_TEL_LEN];
    XU8 imsi[TA_USER_NUM][MAX_IMSI_LEN];
}t_InfectId;

/*TA删除过滤任务时，发消息给注册到TA的模块*/
typedef struct
{
    XU32 traceID;           /*过滤条件对应的TraceID，业务不能直接删除*/
    XU32 index;             /*过滤条件在TraceID中的索引*/
    XU8 tel[MAX_TEL_LEN];    /*过滤任务中的电话号码*/
    XU8 imsi[MAX_IMSI_LEN]; /*过滤任务中的IMSI*/
}t_DelTrace;

/*OAM需要发送如下信息给TA*/
typedef struct
{
    XU16    neType;    /*网元类型*/
    XU16    neID;       /*网元ID*/
    XU16    processID;  /*本进程逻辑进程号*/
    XU16    slotID;     /*本板槽位号*/
    XU16    port;       /*TA与TS通信使用的端口号*/
    XU16    lowSlot;    /*TS低槽位号*/
    XU16    highSlot;   /*TS高槽位号*/
    XU16    tsMode;     /*TS模式:0-单机模式，1-HA模式*/
}t_taCfg;

/*发送给OAM的消息结构*/
typedef struct
{
    XU32 tableID;   /*表ID*/
    XU32 tableNum;  /*表的数量*/
}t_taCfgReq;

/*------------------------------------------------------------------------------*/
#pragma pack(1)

/*命令字类型*/
typedef enum
{
    TA_CMD_GET_IMSI_REQ = 0x0101,      /*ts发给ta，获取imsi*/
    TA_CMD_GET_IMSI_RSP = 0x0102,       /*ta发给ts，获取imsi响应*/
    TA_CMD_GET_TEL_REQ = 0x0103,       /*ts发给ta，获取电话号码*/
    TA_CMD_GET_TEL_RSP = 0x0104,       /*ta发给ts，获取电话号码响应*/
    TA_CMD_START_TRACE = 0x0105,       /*ts发给ta，开始跟踪*/
    TA_CMD_STOP_TRACE = 0x0106,        /*停止跟踪*/
    TA_CMD_TRACE_MSG = 0x0107,         /*跟踪消息*/
    TA_CMD_SYN_INFO_REQ = 0x0108,      /*同步消息请求*/
    TA_CMD_SYN_INFO_RSP = 0x0109,       /*同步消息响应*/
}e_TA_CMD;

/*消息头*/
typedef struct 
{
    XU16   usBeginFlag;              /*起始标识*/
    XU16   usCmd;                    /*命令字*/
    XU16    usBodyLen;                /*消息体长度*/
    XU32   ulSerial;                 /*序列号*/
    XS32   idx;                     /*过滤任务索引号*/
}t_TaMsgHead;


/*结束标志*/
typedef struct 
{
   XU16   usEndFlag;                 /*结束标识*/    
}t_TaMsgTail;


/*获取imsi请求*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];     /*任务的md5*/
    XU16 usTelLen;              /*电话号码长度*/
    XU8 ucTel[MAX_TEL_LEN];     /*电话号码*/
}t_TaGetImsiReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetImsiReqData data;
    t_TaMsgTail tail;
}t_TaGetImsiReq;

/*获取tel请求*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];     /*任务的md5*/
    XU16 usImsiLen;             /*imsi长度*/
    XU8 ucImsi[MAX_IMSI_LEN];   /*imsi*/
}t_TaGetTelReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetTelReqData data;
    t_TaMsgTail tail;
}t_TaGetTelReq;

/*停止跟踪请求*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];
}t_TaStopTraceReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaStopTraceReqData data;
    t_TaMsgTail tail;
}t_TaStopTraceReq;

/*网元信息同步请求*/
typedef struct
{
    t_TaMsgHead head;
    t_TaMsgTail tail;
}t_TaSynInfoReq;

/*开始跟踪*/

/*跟踪类型*/
typedef enum
{
    TRACE_TYPE_USER = 0x01,
    TRACE_TYPE_IF = 0x02,
    TRACE_TYPE_LOG = 0x03,
}e_TRACE_TYPE;

/*用户标识类型*/
typedef enum
{
    USER_INDENT_TYPE_TEL = 0x01,
    USER_INDENT_TYPE_IMSI = 0x02    
}e_USER_INDENT_TYPE;

/*接口类型*/
typedef enum
{    
    IF_TYPE_CX = 0x01,
    IF_TYPE_ISC = 0x02,
    IF_TYPE_MM = 0x03,
    IF_TYPE_MX = 0x04,
    IF_TYPE_MB = 0x05,
    IF_TYPE_PC = 0x06,
    IF_TYPE_PD = 0x07,
    IF_TYPE_PE = 0x08,
    IF_TYPE_PM = 0x09,
    IF_TYPE_PR = 0x0a,
    IF_TYPE_PN = 0x0b,
    IF_TYPE_PH = 0x0c,
    IF_TYPE_S6A = 0x0d,
    IF_TYPE_SH = 0x0e,
    IF_TYPE_SMPP = 0x0f,
    IF_TYPE_S1_EMME = 0x10,
    IF_TYPE_S1_U = 0x11,
    IF_TYPE_S11 = 0x12,
    IF_TYPE_SGI = 0x13,
    IF_TYPE_PE1 = 0x14,
    IF_TYPE_PE2 = 0x15
}e_IF_TYPE;

/*用户标识跟踪的消息类型*/
typedef enum
{
    TRACE_MSG_TYPE_CONTROL = 0x01,  /*控制面*/
    TRACE_MSG_TYPE_VOICE = 0x02,    /*语音*/
    TRACE_MSG_TYPE_VIDEO = 0x03,    /*视频*/
    TRACE_MSG_TYPE_IPDATA = 0x04,    /*IP数据包*/
}e_TRACE_MSG_TYPE;

/*用户跟踪消息体*/
typedef struct 
{
    XU16 usTelLen;           /*电话号码长度*/
    XU8 ucTel[MAX_TEL_LEN];  /*电话号码*/
    XU16 usImsiLen;          /*IMSI长度*/
    XU8 ucImsi[MAX_IMSI_LEN]; /*IMSI*/
    XU8 ucMsgType;           /*消息类型*/
    XU8 ucIfFlag;            /*单用户接口消息*/
    XU8 ucLogLevel;          /*log级别 0~6, oxff*/
}t_TaStarUserTraceReqData;

/*接口跟踪消息体*/
typedef struct 
{
    XU8 ucIfType;           /*接口类型*/
    XU16 usLinkGid;         /*链路组ID*/
    XU16 usLinkId;          /*链路ID*/
}t_TaStarIfTraceReqData;

/*专门用于TA强转以区分TS下发的跟踪类型*/
typedef struct
{
    t_TaMsgHead head;
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*跟踪类型*/
}t_TaTraceType;

typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*跟踪类型*/
    t_TaStarUserTraceReqData task;
}t_TaStartUserTraceReqBody;

typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*跟踪类型*/
    t_TaStarIfTraceReqData task;
}t_TaStartIfTraceReqBody;

/*用户跟踪*/
typedef struct
{
    t_TaMsgHead head;    
    t_TaStartUserTraceReqBody data;
    t_TaMsgTail tail;
}t_TaStartUserTraceReq;

/*接口跟踪*/
typedef struct
{
    t_TaMsgHead head;    
    t_TaStartIfTraceReqBody data;
    t_TaMsgTail tail;
}t_TaStartIfTraceReq;

/*获取imsi响应*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];
    XU16 usImsiLen;             /*imsi长度*/
    XU8 ucImsi[MAX_IMSI_LEN];   /*imsi*/
}t_TaGetImsiRspData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetImsiRspData data;
    t_TaMsgTail tail;
}t_TaGetImsiRsp;


/*获取电话号码响应*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];
    XU16 usTelLen;             /*电话号码长度*/
    XU8 ucTel[MAX_TEL_LEN];    /*电话号码*/
}t_TaGetTelRspData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetTelRspData data;
    t_TaMsgTail tail;
}t_TaGetTelRsp;


/*同步网元信息响应*/
typedef struct
{
    XU16 usNeType;    /*网元类型*/
    XU16 usNeId;      /*网元ID*/
    XU16 usLogicId;   /*逻辑进程ID*/
    XU16 usSlotNum;   /*槽位号*/
}t_TaSynInfoReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaSynInfoReqData data;
    t_TaMsgTail tail;
}t_TaSynInfoRsp;

/*消息体中的公共消息头*/
typedef struct
{
    XU8 ucProVer;     /*协议版本号*/
    XU8 ucMsgDirect;  /*发送方向*/
    XU8 ucTraceType;  /*跟踪类型*/
    XU16 ucNeType;     /*网元类型*/
    XU16 usNeId;      /*网元ID*/
    XU16 usLogicPid;  /*逻辑进程ID*/
    XU32 ulIPAddr;    /*ta IP地址*/
    XU32 ulFID;       /*模块ID*/
    XU32 ulSeqNum;    /*发送序列号*/
    XU64 ulTimeStamp; /*时间戳*/   
}t_TaTraceCommonData;

/*接口消息跟踪响应*/
typedef struct
{
    XU8 ifType;         /*接口类型*/
    XU16 groupID;           /*接口消息源网元标识*/
    XU16 linkID;           /*接口消息目的网元标识*/
    XU16 len;           /*数据长度*/
}t_TaInterfaceRsp;

/*模块接口消息跟踪响应*/
typedef struct
{
    XU32 srcFid;
    XU32 dstFid;
    XU16 len;
}t_TaModIfRsp;

/*log数据跟踪响应*/
typedef struct
{
    XU32 logID;         /*log跟踪ID*/
    XU32 line;          /*行号*/
    XU16 level;         /*日志级别*/
    XU16 len;           /*log内容长度*/
}t_TaLogRsp;
#pragma pack()
/*-------------------------------------------------------------------------
                   模块外部接口函数
-------------------------------------------------------------------------*/

/************************************************************************
* 业务模块删除本模块保存的TraceID的回调函数的定义
* 输入：id   - 要删除的TraceID
* 返回 : 
************************************************************************/
typedef XS32 (*DelFun) (XU32 id);


XS8 TA_Init(XVOID *p1, XVOID *p2);
XS8 TA_timerProc( t_BACKPARA* pParam);
XS8 TA_msgProc(XVOID* pMsgP, XVOID*sb );
XU32 XOS_TaInfect(const t_InfectId *userID);
XS32 XOS_TaTrace(XU32 fid,e_TaDirection direct,const t_XOSCOMMHEAD *msg);
XS32 XOS_TaDataTrace(XU32 srcFid,XU32 dstFid,const XVOID *data,XU32 len,const XU8 *id,e_IdType type,e_TaDirection direction);
XS32 XOS_TmgFilter(XU32 fid,const XVOID *data,XU32 len,e_TRACE_MSG_TYPE msgType,XU32 traceID);
XS32 XOS_TaRegDel(XU32 fid);
XS32 XOS_TaInterface(XU32 fid,e_IF_TYPE type,XU16 groupID,XU16 linkID,e_TaDirection direct,t_TaAddress *addr,const XVOID *data,XU16 len);
XS32 XOS_TaIdTrace(const t_XOSCOMMHEAD *msg,const XU8 *id,e_IdType type,e_TaDirection direction);
XU32 XOS_LogInfect(const t_InfectId *userID);
XVOID XOS_LogTrace(const XCHAR* FileName, XU32 ulLineNum,const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel,XU32 logID, const XCHAR *cFormat, ... );
XVOID XOS_LogTraceX(const XCHAR* FileName, XU32 ulLineNum,const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel,XU32 logID, const XCHAR *cFormat, va_list ap );

XVOID XOS_SetInterfaceVer(XU8 ver);

#endif  /*#ifdef XOS_TRACE_AGENT*/

#ifdef __cplusplus
}
#endif /* _ _cplusplus */
#endif /* _TRACE_AGENT_H_ */

