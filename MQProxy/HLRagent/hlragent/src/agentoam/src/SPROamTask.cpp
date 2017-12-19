/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     eHssOamTask.cpp
* Author:       luhaiyan
* Date：        30-08-2014
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
#include "SPROamTask.h"
#include "agentinclude.h"

#include "oam_main.h"
#include "SPROamCli.h"
#include "agentDbTask.h"
#include "SPRCommFun.h"
#include "agentuaOam.h"







/**************************************************************************
函 数 名: SPROamInitProc
函数功能: 初始化接口,向平台注册
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS8 agent_OamInitProc(XVOID *pPara1, XVOID *Para2)
{
	XS32 ret = 0;
	//注册oamcli命令行
	RegAgentOamCliCmd();



    XOS_RegAppVer("HLRagent", "R001B16D31SP05");
    //R001B16D31SP04  查询主被叫分离

    return XSUCC;
}

/**************************************************************************
函 数 名: SPROamMsgProc
函数功能: 消息处理接口,向平台注册
参    数:
返 回 值:
**************************************************************************/
XS8 agent_OamMsgProc(XVOID *msg, XVOID *pPara)
{

    XS8 ret = XERROR;
	t_XOSCOMMHEAD* pxosMsg = (t_XOSCOMMHEAD*)msg;
	AGT_OAM_CFG_REQ_T* pRecvMsg = XNULL;

	if(XNULL == msg)
    {
		return XERROR;
    }

	pRecvMsg = (AGT_OAM_CFG_REQ_T *)pxosMsg->message;

	switch(pRecvMsg->uiTableId)
	{


    	case MML_UA_LINK_TABLE_ID:
    		ret = agentUA_LinkHandler(pRecvMsg);
    		break;


    	default:
    		XOS_PRINT(MD(FID_UA,PL_ERR),"oam_XosMsgProc recv msg from unknown MML table id %u",pRecvMsg->uiTableId);
    		ret = XERROR;
    		break;
	}
    
    return XSUCC;
}

/**************************************************************************
函 数 名: SPROamTimeoutProc
函数功能: 超时处理接口,向平台注册
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 agent_OamTimeoutProc(t_BACKPARA  *pstPara)
{
    return XSUCC;
}

/**************************************************************************
函 数 名: SPROamNoticeProc
函数功能: 通知处理函数，向平台注册
参    数:
返 回 值:
**************************************************************************/
XS8 agent_OamNoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{	

	



	return XSUCC;
}







