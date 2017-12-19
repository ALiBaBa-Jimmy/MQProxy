/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentHlrCom.h
功    能: agentHlr 消息处理相关
时    间: 2012年8月29日
**************************************************************************/
#pragma once

#include "xostype.h"
#include "agentHlrTask.h"
#include "fid_def.h"
#include "agentTrace.h"
#include "agentDB_API.h"
#include "MysqlDB_API.h"
#include "smu_sj3_type.h"
#include "agentHDB_API.h"

#include <map>
using  std::map;
#include <string>
using  std::string;


#ifdef __cplusplus
extern "C" {
#endif



#pragma pack(1)

typedef enum
{

    e_Res_XSUCC = 0,
    e_Res_Home_QryFail,
    e_Res_QryRouteFail,
    e_Res_Qry_Result_BUTT
    

}E_QRYALL_RESULT;


#pragma pack()
XS32 agentHLR_DBTest(XU32 UID);


XS32 agentHLR_SendHlrMsg(t_AGENTUA* pInerMsg, XU32 destFID);

XS32 agentHLR_EncodeTLV(XU8 *buf, XU8 tag, XVOID *Value, XU32 valueLen);
XS32 agentHLR_DecodeTLV(XU8 *buf, XU32 buflen,  XU8 tag, XVOID *value);
XS32 agentHLR_DecodeULreg(XU8 *buf, XU32 buflen, SUpdateUserInfoReq *pRegInfo);
XS32 agentHLR_UpdateAuthMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_DecodeSmcReport(XU8 *buf, XU32 buflen, tRep2Agent *pReportInfo);

XS32 agentHLR_UpdateHomeMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_ProHlrReqFromMqtt(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_ProHlrRspFromUa(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_ProHlrRspFromMqtt(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_ProHlrReqFromUa(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);



XS32 agentHLR_QryUIDRequestMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
//
XS32 OperType_QryAllInfoRsptoUA(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 OperType_Push(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen, XCHAR *topic);
XS32 OperType_SubscribeorNot(XU32 fid,  XCHAR *topic, XU16 msgID);

XS32 OperType_QryAllInfoRspPush(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 OperType_ProcQryAllInfoReq(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 OperType_ProcULReq(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 OperType_ProEsmcReport(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 OperType_TransmittoUA(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XVOID agentHLR_PrintULInfo(XU32 fid, SUpdateUserInfoReq *pNew, TXdbHdbDyn *pOld);
XS32 PushUserInfo( XU8 * pUid, XU8* topic);
XS32 Push2LocalHlrUpdateStatus(tRep2Agent* pInfo, XU8 * pTopic);
XS32 agentHLR_GetValue(XU8* pOut, XU8* pInMsg);
XU64 GetPresentTime(XVOID);
XS32 agentHLR_GetCurSysTime(XS8 *pTimeStr);
XS32 agentHLR_TimeMm2Str(XU64 time, XS8 *pTimeStr);
XS32 agentHLR_ChrToHex(XU8 chr);
XS32 agentHLR_StrToHex(XU8 * pStr,XU8 *pHex,XU64 ulLen);
XS32 agentHLR_HexToStr(XU8 *pHex,XU8 * pStr,XU64 ulLen);

XS32 agentHLR_QryCalledTelMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);

#ifdef __cplusplus
}
#endif 




