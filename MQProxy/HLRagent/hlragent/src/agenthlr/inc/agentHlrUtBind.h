/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentHlrCom.h
功    能: agentHlr 消息处理相关
时    间: 2012年8月29日
**************************************************************************/
#pragma once

#include "xostype.h"
#include "smu_sj3_type.h"
#include "agentHlrCom.h"


#define TIMER_TYPE_AGENT_UTBIND_SYN   0X03
#define TIMER_LENGTH_AGENT_UTINFO  60*1000 /*租赁关系同步时长*/

#define AGENT_SYNUTINFO_MAXNUM   20

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(1)

typedef struct
{
	XU8 pid[LENGTH_OF_PID];
    XU8 port;
}SPidPort;






#pragma pack()
XS32 agentHLR_UtBindInfoInit(XVOID);
XVOID agentHLR_InsrtCallback(SLeaseHoldCtlTable *pIns);
XS32 agentHLR_Syn2HlrUtInfo(XVOID);
XS32 agentHLR_ProcUtLeaseMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen);
XS32 agentHLR_GetUtInfo(XVOID);
XS32 agentHLR_UtInfoInit(XVOID);
XS32 agentHLR_UtInfoTimerSynPro(t_BACKPARA *tBackPara);

#ifdef __cplusplus
}
#endif 




