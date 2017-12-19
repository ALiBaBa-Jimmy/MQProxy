/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: agentHlrCom.h
��    ��: agentHlr ��Ϣ�������
ʱ    ��: 2012��8��29��
**************************************************************************/
#pragma once

#include "xostype.h"
#include "smu_sj3_type.h"




#ifdef __cplusplus
extern "C" {
#endif

/*��ѯ���ƿ�hash��С*/
#define SCU_MAX_NUM_OF_QRYFSM             5000

/*tel��uidӳ���ϵhash��С*/
#define SCU_MAX_NUM_OF_TELMAP             150000


#define LENGTH_OF_NICK_NAME     18
#define AGENT_TCPE_MAX_PACKNUM   2
#pragma pack(1)
typedef struct
{
	XU8 pid[LENGTH_OF_PID];
    XU8 port;
}tPidPort;
/*��ѯȺ��*/
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
    XU64 qrytime;/*ʱ���*/
    XU8  srcNetID[LENGTH_OF_NETWORK_ID];/*������Ϣ������ID*/
    XU16 ccbIndex;

    PTIMER  Timer;/*��ʱ��*/

    tUserInfoBuf buf;
    

}tUSM;

typedef struct
{
    XU64 updatetime;  /*���һ�θ���ʱ��*/
    XU64 LastSriTime;/*��һ�κ���ʱ��*/
    XU8  uid[LENGTH_OF_UID];
    XU8 result;
    TXdbHashStat   tHashStat;
    TXdbHashDyn    tHashDyn ;
    TXdbHashSS     tHashSS  ;
}tContext;

/*�ڴ���������ʱ�����ýṹ*/
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




