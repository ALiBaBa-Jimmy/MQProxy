/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��: agentProtocol.h
��    ��: ��������smppЭ��ջ
ʱ    ��: 2012��9��13��
**************************************************************************/
#pragma once


#include "xosshell.h"
#include "agentuaCommon.h"


#define XS64_SIZE 8
#define ENQUIRELINK_INTERVAL_TIME_OUT     5000     //EnquireLink���
#define ENQUIRELINK_RSP_TIME_OUT          1000     //EnquireLink��Ӧ���
/**************************************************************************
�� �� ��: smppUA_NtlRspProc
��������: NTL��Ϣ������
��    ��:
�� �� ֵ:
**************************************************************************/
AGENTUA_API XS32 agentUA_NtlRspProc(t_XOSCOMMHEAD *pMsgHead);
AGENTUA_API XS32 agentUA_HeartSendReq(XU32 fid, XS32 appHandle);

AGENTUA_API XVOID agentUA_MsgBufferMatch(HLINKHANDLE linkHandle, XS32 *iMsgBufferIndex);




