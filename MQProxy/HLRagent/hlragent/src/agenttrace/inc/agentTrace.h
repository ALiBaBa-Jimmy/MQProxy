/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssTraceApi.h
* Author:       luhaiyan
* Date:         09-07-2014
* OverView:     eHSS TRACE接口
*
* History:      最新历史修改在最前面
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*
* Revisor:      修改者姓名
* Date:         MM-DD-YYYY
* Description:  描述修改功能点
*******************************************************************************/
#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#include "trace_agent.h"
#include "SPRTraceStruct.h"

/**************************************************************************
* 函 数 名： agentTrace
* 函数功能： HSS的trace函数
* 输    入： ulFid           - 输入，功能块ID
*			 ulLevel         - 输入，打印级别
*            const t_InfectId *userID：要过滤的信息
*			 ucFormat        - 输入，打印格式化字符串
*			 ...             - 输入，打印参数
* 输    出：
* 返 回 值：
* 说    明：
**************************************************************************/
XVOID agentTrace( SSPRTracePre* pSprTracePre, const XCHAR* FileName, XU32 ulLineNum, const XCHAR* FunName, XU32 ulFid, e_PRINTLEVEL eLevel, 				
				const XCHAR *cFormat, ... );

/**************************************************************************
* 函 数 名： SYS_MsgSend
* 函数功能：  SYS_MsgSend -封装平台的消息发送函数，为了监视使用
* 输    入：
* 输    出：
* 返 回 值：
* 说    明：
**************************************************************************/
XS32 SYS_MsgSend(t_XOSCOMMHEAD *pMsg);

#ifdef __cplusplus
}
#endif /* __cplusplus */




