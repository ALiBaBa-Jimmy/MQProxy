/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentHlrCom.h
功    能: agentHlr 消息处理相关
时    间: 2012年8月29日
**************************************************************************/
#pragma once

#include "xostype.h"
#include "smu_sj3_type.h"




#ifdef __cplusplus
extern "C" {
#endif

/*查询控制块hash大小*/
#define SCU_MAX_NUM_OF_QRYFSM             5000

/*tel和uid映射关系hash大小*/
#define SCU_MAX_NUM_OF_TELMAP             150000


#define LENGTH_OF_NICK_NAME     18
#define AGENT_TCPE_MAX_PACKNUM   2
#pragma pack(1)
typedef struct
{
	XU8 pid[LENGTH_OF_PID];
    XU8 port;
}tPidPort;
/*查询群组*/
typedef struct
{
    XU32   gid;
    XU8    ext[LENGTH_OF_EXT];
    
}TXdbScuCtrxReq;

typedef struct
{
    XU8 buf[2000];
    XU32 bufLen;

}tTcpeBuf;
typedef struct
{
    tTcpeBuf singleBuf[AGENT_TCPE_MAX_PACKNUM];
    XU8 endPackId;
}tUserInfoBuf;

typedef struct
{
    XU8  qrytype;
    XU8  operatorTyp;
	XU8 uid[LENGTH_OF_UID];
    XU8 tel[LENGTH_OF_TEL];
    TXdbScuCtrxReq  ctrxreq;
    XU8 UserBM[LENGTH_OF_NICK_NAME];
    tPidPort pidport;    

    XU8  state;
    XU64 qrytime;/*时间戳*/
    XU8  srcNetID[LENGTH_OF_NETWORK_ID];/*请求消息的网络ID*/
    XU16 ccbIndex;

    PTIMER  Timer;/*定时器*/

    tUserInfoBuf buf;
    

}tUSM;

typedef struct
{
    XU64 updatetime;  /*最近一次更新时间*/
    XU64 LastSriTime;/*上一次呼叫时间*/
    XU8  uid[LENGTH_OF_UID];
    XU8 result;
    TXdbHashStat   tHashStat;
    TXdbHashDyn    tHashDyn ;
    TXdbHashSS     tHashSS  ;
}tContext;

/*内存数据清理时间配置结构*/
typedef struct
{
    XU16 hour;
    XU16 minitus;

}tClearContextTime;



#pragma pack()
extern XU64 g_UpdateRequestTimeOut;
XS32 agentHLR_UserInfoInit(XVOID);
XS32 agentHLR_TimerUserInfoPro(t_BACKPARA *tBackPara);
XS32 agentHLR_UserInfoCheckPro(t_BACKPARA *tBackPara);

XU32 UserInfo_size(XVOID);
XS32 UserInfo_Insert(XU8* uid, tContext* pContext);
XS32 UserInfo_Deletebyuid(XU8* uid);
XS32 UserInfo_Qrybyuid(XU8* uid, tContext* pResult);
XS32 UserInfo_Qry(tUSM *pQryInfo, tContext* pResult);

XS32 UserHash_DelbyccbIndex(XU16 ccbIndex);
XU32 GetHlrAgentCcbIndex();

XS32 DelTelUidHash(XU8 *pDelTelno,XU8 *pDelUid);
XS32 AddTelUidHash(XU8 *pAddTelno,XU8 *pAddUid);
XS32 DelTelUidHashTelLst(SSubTelnoInfoList* pTelLst, XU8* uid);
XS32 AddTelUidHashTelLst(SSubTelnoInfoList* pTelLst, XU8* uid);
tContext*  UserInfo_Qrybyuid_Ext(XU8* uid);
    


#ifdef __cplusplus
}
#endif 




