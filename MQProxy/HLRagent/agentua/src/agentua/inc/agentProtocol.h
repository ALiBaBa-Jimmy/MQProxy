/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名: agentProtocol.h
功    能: 向下适配smpp协议栈
时    间: 2012年9月13日
**************************************************************************/
#pragma once


#include "xosshell.h"
#include "agentuaCommon.h"


#define XS64_SIZE 8
#define ENQUIRELINK_INTERVAL_TIME_OUT     5000     //EnquireLink间隔
#define ENQUIRELINK_RSP_TIME_OUT          1000     //EnquireLink响应间隔
/**************************************************************************
函 数 名: smppUA_NtlRspProc
函数功能: NTL消息处理函数
参    数:
返 回 值:
**************************************************************************/
AGENTUA_API XS32 agentUA_NtlRspProc(t_XOSCOMMHEAD *pMsgHead);
AGENTUA_API XS32 agentUA_HeartSendReq(XU32 fid, XS32 appHandle);

AGENTUA_API XVOID agentUA_MsgBufferMatch(HLINKHANDLE linkHandle, XS32 *iMsgBufferIndex);




