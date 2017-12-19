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
                  ����ͷ�ļ�
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
                  �궨��
--------------------------------------------------------------*/
#define TA_CFG_REQ  0x0201    /*TA��OAM����t_taCfg��Ϣ����ϢID*/
#define TA_CFG_RSP  0x0202    /*OAM��TA��Ӧt_taCfg��Ϣ����ϢID*/

#define MME_IMSI_REQ    0x0301  /*TA��eMME���͵�����IMSI����Ϣ*/
#define MME_IMSI_RSP    0x0302  /*eMME��TA���͵�IMSI����Ӧ��Ϣ*/
#define MME_TEL_REQ     0x0303  /*TA��eMME���͵�����绰�������Ϣ*/
#define MME_TEL_RSP     0x0304  /*eMME��TA���͵ĵ绰�������Ӧ��Ϣ*/
#define MME_SAVE_IDENTIFY       0x0305      /*NAS��Ҫ������S1APֱ�ӵļ�Ȩ��Ϣ*/
#define MME_NOT_SAVE_IDENTIFY   0x0306      /*NASֹͣ������S1APֱ�ӵļ�Ȩ��Ϣ*/
#define TA_DEL_REQ     0x0307  /*TA����Ԫģ�鷢�͵�ɾ��TraceID��Ϣ*/

#define MAX_TEL_LEN (32)
#define MAX_IMSI_LEN (15)
#define MAX_MD5_LEN (32)

#define TA_USER_NUM     2       /*��Ⱦʱÿ�����͵��û���ʶ֧�ֵ��������*/
#define TA_MAX_MSG_LEN  4096    /*TA���͸�TS�����trace���ݳ���*/
/*-------------------------------------------------------------------------
                     ģ���ⲿ�ṹ��ö�ٶ���
-------------------------------------------------------------------------*/
/*TS����ģʽ*/
typedef enum
{
    e_SingleMode    = 0x0000,
    e_HaMode        = 0x0001
}e_TsMode;

/*�û���ʶ���Ͷ���*/
typedef enum
{
    e_TA_TEL = 1,  /*�绰����*/
    e_TA_IMSI      /*IMSI*/
}e_IdType;

/*�ӿ���Ϣ����*/
typedef enum
{
    e_TA_SEND = 0x01,     /*���˷���*/
    e_TA_RECV = 0x02         /*���˽���*/
}e_TaDirection;

/*�ӿڹ���ʱ��ԴĿ�ĵ�ַ����*/
typedef struct
{
    t_IPADDR src;
    t_IPADDR dst;
}t_TaAddress;

/*ҵ���Ⱦ�ӿڲ��������ͬʱ֧�������绰���������IMSI*/
typedef struct
{
    XU16 idNum;
    XU16 imsiNum;
    XU8 id[TA_USER_NUM][MAX_TEL_LEN];
    XU8 imsi[TA_USER_NUM][MAX_IMSI_LEN];
}t_InfectId;

/*TAɾ����������ʱ������Ϣ��ע�ᵽTA��ģ��*/
typedef struct
{
    XU32 traceID;           /*����������Ӧ��TraceID��ҵ����ֱ��ɾ��*/
    XU32 index;             /*����������TraceID�е�����*/
    XU8 tel[MAX_TEL_LEN];    /*���������еĵ绰����*/
    XU8 imsi[MAX_IMSI_LEN]; /*���������е�IMSI*/
}t_DelTrace;

/*OAM��Ҫ����������Ϣ��TA*/
typedef struct
{
    XU16    neType;    /*��Ԫ����*/
    XU16    neID;       /*��ԪID*/
    XU16    processID;  /*�������߼����̺�*/
    XU16    slotID;     /*�����λ��*/
    XU16    port;       /*TA��TSͨ��ʹ�õĶ˿ں�*/
    XU16    lowSlot;    /*TS�Ͳ�λ��*/
    XU16    highSlot;   /*TS�߲�λ��*/
    XU16    tsMode;     /*TSģʽ:0-����ģʽ��1-HAģʽ*/
}t_taCfg;

/*���͸�OAM����Ϣ�ṹ*/
typedef struct
{
    XU32 tableID;   /*��ID*/
    XU32 tableNum;  /*�������*/
}t_taCfgReq;

/*------------------------------------------------------------------------------*/
#pragma pack(1)

/*����������*/
typedef enum
{
    TA_CMD_GET_IMSI_REQ = 0x0101,      /*ts����ta����ȡimsi*/
    TA_CMD_GET_IMSI_RSP = 0x0102,       /*ta����ts����ȡimsi��Ӧ*/
    TA_CMD_GET_TEL_REQ = 0x0103,       /*ts����ta����ȡ�绰����*/
    TA_CMD_GET_TEL_RSP = 0x0104,       /*ta����ts����ȡ�绰������Ӧ*/
    TA_CMD_START_TRACE = 0x0105,       /*ts����ta����ʼ����*/
    TA_CMD_STOP_TRACE = 0x0106,        /*ֹͣ����*/
    TA_CMD_TRACE_MSG = 0x0107,         /*������Ϣ*/
    TA_CMD_SYN_INFO_REQ = 0x0108,      /*ͬ����Ϣ����*/
    TA_CMD_SYN_INFO_RSP = 0x0109,       /*ͬ����Ϣ��Ӧ*/
}e_TA_CMD;

/*��Ϣͷ*/
typedef struct 
{
    XU16   usBeginFlag;              /*��ʼ��ʶ*/
    XU16   usCmd;                    /*������*/
    XU16    usBodyLen;                /*��Ϣ�峤��*/
    XU32   ulSerial;                 /*���к�*/
    XS32   idx;                     /*��������������*/
}t_TaMsgHead;


/*������־*/
typedef struct 
{
   XU16   usEndFlag;                 /*������ʶ*/    
}t_TaMsgTail;


/*��ȡimsi����*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];     /*�����md5*/
    XU16 usTelLen;              /*�绰���볤��*/
    XU8 ucTel[MAX_TEL_LEN];     /*�绰����*/
}t_TaGetImsiReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetImsiReqData data;
    t_TaMsgTail tail;
}t_TaGetImsiReq;

/*��ȡtel����*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];     /*�����md5*/
    XU16 usImsiLen;             /*imsi����*/
    XU8 ucImsi[MAX_IMSI_LEN];   /*imsi*/
}t_TaGetTelReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetTelReqData data;
    t_TaMsgTail tail;
}t_TaGetTelReq;

/*ֹͣ��������*/
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

/*��Ԫ��Ϣͬ������*/
typedef struct
{
    t_TaMsgHead head;
    t_TaMsgTail tail;
}t_TaSynInfoReq;

/*��ʼ����*/

/*��������*/
typedef enum
{
    TRACE_TYPE_USER = 0x01,
    TRACE_TYPE_IF = 0x02,
    TRACE_TYPE_LOG = 0x03,
}e_TRACE_TYPE;

/*�û���ʶ����*/
typedef enum
{
    USER_INDENT_TYPE_TEL = 0x01,
    USER_INDENT_TYPE_IMSI = 0x02    
}e_USER_INDENT_TYPE;

/*�ӿ�����*/
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

/*�û���ʶ���ٵ���Ϣ����*/
typedef enum
{
    TRACE_MSG_TYPE_CONTROL = 0x01,  /*������*/
    TRACE_MSG_TYPE_VOICE = 0x02,    /*����*/
    TRACE_MSG_TYPE_VIDEO = 0x03,    /*��Ƶ*/
    TRACE_MSG_TYPE_IPDATA = 0x04,    /*IP���ݰ�*/
}e_TRACE_MSG_TYPE;

/*�û�������Ϣ��*/
typedef struct 
{
    XU16 usTelLen;           /*�绰���볤��*/
    XU8 ucTel[MAX_TEL_LEN];  /*�绰����*/
    XU16 usImsiLen;          /*IMSI����*/
    XU8 ucImsi[MAX_IMSI_LEN]; /*IMSI*/
    XU8 ucMsgType;           /*��Ϣ����*/
    XU8 ucIfFlag;            /*���û��ӿ���Ϣ*/
    XU8 ucLogLevel;          /*log���� 0~6, oxff*/
}t_TaStarUserTraceReqData;

/*�ӿڸ�����Ϣ��*/
typedef struct 
{
    XU8 ucIfType;           /*�ӿ�����*/
    XU16 usLinkGid;         /*��·��ID*/
    XU16 usLinkId;          /*��·ID*/
}t_TaStarIfTraceReqData;

/*ר������TAǿת������TS�·��ĸ�������*/
typedef struct
{
    t_TaMsgHead head;
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*��������*/
}t_TaTraceType;

typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*��������*/
    t_TaStarUserTraceReqData task;
}t_TaStartUserTraceReqBody;

typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];          /*md5*/
    XU8 ucTraceType;                 /*��������*/
    t_TaStarIfTraceReqData task;
}t_TaStartIfTraceReqBody;

/*�û�����*/
typedef struct
{
    t_TaMsgHead head;    
    t_TaStartUserTraceReqBody data;
    t_TaMsgTail tail;
}t_TaStartUserTraceReq;

/*�ӿڸ���*/
typedef struct
{
    t_TaMsgHead head;    
    t_TaStartIfTraceReqBody data;
    t_TaMsgTail tail;
}t_TaStartIfTraceReq;

/*��ȡimsi��Ӧ*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];
    XU16 usImsiLen;             /*imsi����*/
    XU8 ucImsi[MAX_IMSI_LEN];   /*imsi*/
}t_TaGetImsiRspData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetImsiRspData data;
    t_TaMsgTail tail;
}t_TaGetImsiRsp;


/*��ȡ�绰������Ӧ*/
typedef struct
{
    XU8 ucMd5[MAX_MD5_LEN];
    XU16 usTelLen;             /*�绰���볤��*/
    XU8 ucTel[MAX_TEL_LEN];    /*�绰����*/
}t_TaGetTelRspData;

typedef struct
{
    t_TaMsgHead head;
    t_TaGetTelRspData data;
    t_TaMsgTail tail;
}t_TaGetTelRsp;


/*ͬ����Ԫ��Ϣ��Ӧ*/
typedef struct
{
    XU16 usNeType;    /*��Ԫ����*/
    XU16 usNeId;      /*��ԪID*/
    XU16 usLogicId;   /*�߼�����ID*/
    XU16 usSlotNum;   /*��λ��*/
}t_TaSynInfoReqData;

typedef struct
{
    t_TaMsgHead head;
    t_TaSynInfoReqData data;
    t_TaMsgTail tail;
}t_TaSynInfoRsp;

/*��Ϣ���еĹ�����Ϣͷ*/
typedef struct
{
    XU8 ucProVer;     /*Э��汾��*/
    XU8 ucMsgDirect;  /*���ͷ���*/
    XU8 ucTraceType;  /*��������*/
    XU16 ucNeType;     /*��Ԫ����*/
    XU16 usNeId;      /*��ԪID*/
    XU16 usLogicPid;  /*�߼�����ID*/
    XU32 ulIPAddr;    /*ta IP��ַ*/
    XU32 ulFID;       /*ģ��ID*/
    XU32 ulSeqNum;    /*�������к�*/
    XU64 ulTimeStamp; /*ʱ���*/   
}t_TaTraceCommonData;

/*�ӿ���Ϣ������Ӧ*/
typedef struct
{
    XU8 ifType;         /*�ӿ�����*/
    XU16 groupID;           /*�ӿ���ϢԴ��Ԫ��ʶ*/
    XU16 linkID;           /*�ӿ���ϢĿ����Ԫ��ʶ*/
    XU16 len;           /*���ݳ���*/
}t_TaInterfaceRsp;

/*ģ��ӿ���Ϣ������Ӧ*/
typedef struct
{
    XU32 srcFid;
    XU32 dstFid;
    XU16 len;
}t_TaModIfRsp;

/*log���ݸ�����Ӧ*/
typedef struct
{
    XU32 logID;         /*log����ID*/
    XU32 line;          /*�к�*/
    XU16 level;         /*��־����*/
    XU16 len;           /*log���ݳ���*/
}t_TaLogRsp;
#pragma pack()
/*-------------------------------------------------------------------------
                   ģ���ⲿ�ӿں���
-------------------------------------------------------------------------*/

/************************************************************************
* ҵ��ģ��ɾ����ģ�鱣���TraceID�Ļص������Ķ���
* ���룺id   - Ҫɾ����TraceID
* ���� : 
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

