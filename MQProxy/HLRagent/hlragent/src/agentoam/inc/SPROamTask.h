/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssOamTask.h
* Author:       luhaiyan
* Date:        30-08-2014
* OverView:     eHSSOAM模块的任务定义
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

#include "xosshell.h"

//#include "diamuaOamStruct.h"
#define HA_STATUS_NOTIFY_MSG_ID 20
//#define ALARM_NOTIFY_MSG_ID 21
#ifdef __cplusplus
extern "C" {
#endif


/**************************************************************************
函 数 名: SPROamInitProc
函数功能: 初始化接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 agent_OamInitProc(XVOID *pPara1, XVOID *pPara2 );

/**************************************************************************
函 数 名: SPROamMsgProc
函数功能: 消息处理接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 agent_OamMsgProc(XVOID *msg, XVOID *pPara);

/**************************************************************************
函 数 名: SPROamTimeoutProc
函数功能: 超时处理接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 agent_OamTimeoutProc(t_BACKPARA  *pstPara);

/**************************************************************************
函 数 名: SPROamNoticeProc
函数功能: 通知处理函数，向平台注册
参    数:
返 回 值:
**************************************************************************/
 XS8 agent_OamNoticeProc(XVOID* pLVoid, XVOID* pRVoid);












#ifdef __cplusplus
}
#endif /* __cplusplus */




