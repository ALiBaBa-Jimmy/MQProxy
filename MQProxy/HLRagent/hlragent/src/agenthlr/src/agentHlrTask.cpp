/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年10月17日
**************************************************************************/
#include "agentHlrTask.h"
#include "fid_def.h"
#include "agentTrace.h"
#include "smu_sj3_type.h"
#include "agentuaOam.h"
#include "agentHlrCom.h"
#include "clishell.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"
#include "agentHlr_Cmd.h"
#include "agentHlrUtBind.h"



#define  ___MESSAGE__FROM__MQTT_
XS32 agentHLR_ProHlrMsgFromMqtt(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    XU8 ucPackType = pHlrMsg->Ph.ucPackType;
    
    XOS_Trace(MD(FID_HLR, PL_DBG), "Enter ucPackType = %u.", ucPackType);
    switch(ucPackType)
    {
        case SJ_REQ:
          
            ret = agentHLR_ProHlrReqFromMqtt(pHlrMsg, agentUaMsgLen);
            break;
        case SJ_RSP:

            ret = agentHLR_ProHlrRspFromMqtt(pHlrMsg, agentUaMsgLen);
            break;  

        default:
		    XOS_Trace(MD(FID_HLR,PL_ERR), "ucPackType = %u is Err!", ucPackType);
            break;

    }

    
   
    return ret;
}
/*****************************************************************************
 Prototype    : agentHLR_ProMqttXosMsg
 Description  : 处理来自mqtt模块的消息
 Input        : t_XOSCOMMHEAD* pXosMsg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS8 agentHLR_ProMqttXosMsg(t_XOSCOMMHEAD* pXosMsg)
{
	t_AGENTUA* pAgentUaMsg = XNULL;
    t_Sj3_Oper_Rsp *pHlrMsg = NULL;
	XS32 ret;
	

	pAgentUaMsg = (t_AGENTUA*)pXosMsg->message;
	if (pAgentUaMsg == XNULL)
	{
		XOS_PRINT(MD(FID_HLR, PL_ERR), "agentHLR_ProUAXosMsg Receive NULL Message!");
		return XERROR;
	}

	
	 
    pHlrMsg = (t_Sj3_Oper_Rsp*)pAgentUaMsg->pData;
    if(NULL == pHlrMsg)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "pHlrMsg is NULL Message!");
        return XERROR;
    }
    
	ret = agentHLR_ProHlrMsgFromMqtt(pHlrMsg, pAgentUaMsg->msgLenth);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " error!");
        
    }
    XOS_MemFree(FID_HLR, pHlrMsg);
    pHlrMsg = NULL;
    
    return XSUCC;
}


#define  ___MESSAGE__FROM__UA_
XS32 agentHLR_ProHlrMsgFromUA(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    XS32 ret = XERROR;
    

    XU8 ucPackType = pHlrMsg->Ph.ucPackType;    
    XOS_Trace(MD(FID_HLR, PL_DBG), "Enter ucPackType = %u.", ucPackType);
    switch(ucPackType)
    {
        case SJ_REQ:
            ret = agentHLR_ProHlrReqFromUa( pHlrMsg, agentUaMsgLen);
            break;
        case SJ_RSP:


            ret = agentHLR_ProHlrRspFromUa( pHlrMsg, agentUaMsgLen);
            break;  

        default:
		    XOS_Trace(MD(FID_HLR,PL_ERR), "ucPackType = %u is Err!", ucPackType);
            break;

    }

    
   
    return ret;
}
/*****************************************************************************
 Prototype    : agentHLR_ProUAXosMsg
 Description  : 处理来自ua模块的消息
 Input        : t_XOSCOMMHEAD* pXosMsg  
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS8 agentHLR_ProUAXosMsg(t_XOSCOMMHEAD* pXosMsg)
{
	t_AGENTUA* pAgentUaMsg = XNULL;
    t_Sj3_Oper_Rsp *pHlrMsg = NULL;
	XS32 ret;
	

	pAgentUaMsg = (t_AGENTUA*)pXosMsg->message;
	if (pAgentUaMsg == XNULL)
	{
		XOS_PRINT(MD(FID_HLR, PL_ERR), "agentHLR_ProUAXosMsg Receive NULL Message!");
		return XERROR;
	}

	
	 
    pHlrMsg = (t_Sj3_Oper_Rsp*)pAgentUaMsg->pData;
    if(NULL == pHlrMsg)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "pHlrMsg is NULL Message!");
        return XERROR;
    }
    
	ret = agentHLR_ProHlrMsgFromUA(pHlrMsg, pAgentUaMsg->msgLenth);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " error!");
        
    }
    
    XOS_MemFree(FID_HLR, pHlrMsg);
    pHlrMsg = NULL;
    return ret;
}
#define   _____XOS_PLATEFORM____


/**************************************************************************
函 数 名: agentHLR_InitProc
函数功能: 协议栈初始化函数,对协议栈进行初始化,同时注册OAM回调接口
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS8 agentHLR_InitProc(XVOID *pPara1, XVOID *Para2)
{

	XS32 RetValue;

    agentHDB_SubLoadUtBindCallback(agentHLR_InsrtCallback);

    
    /*用户缓存部分初始化*/
    RetValue = agentHLR_UserInfoInit();
    if(XSUCC != RetValue)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " User info Module init failed!");
        return XERROR;
    }

    /*telnet 命令初始化*/
    RetValue = agentHLR_CmdInit();
    if(XSUCC != RetValue)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " Cmd Module init failed!");
        return XERROR;
    }
#if 0
    RetValue = agentHLR_UtInfoInit();
    if(XSUCC != RetValue)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " Ut Lease Module init failed!");
        return XERROR;
    }
#endif
    return XSUCC;
}

/**************************************************************************
函 数 名: agentHLR_XosMsgProc
函数功能: XOS平台消息处理函数,用于处理来至XOS平台的消息
参    数:
返 回 值:
**************************************************************************/
XS8 agentHLR_XosMsgProc(XVOID *msg, XVOID *pPara)
{

    XS8 ret = XERROR;
    t_XOSCOMMHEAD* pxosMsg = (t_XOSCOMMHEAD*)msg;

	if(XNULL == msg)
    {
        XOS_Trace(MD(FID_HLR,PL_ERR),"agentHLR_XosMsgProc input is NULL");
		return XERROR;
    }
	XOS_Trace(MD(FID_HLR,PL_DBG),"FID_HLR recv msg Fid = %d", pxosMsg->datasrc.FID);
    switch(pxosMsg->datasrc.FID)
    {
	    case FID_MQTT:
            /*经过MQTTserver 推送上来的消息 需要发送到hlr/UDC网元*/
			ret = agentHLR_ProMqttXosMsg(pxosMsg);
			break;
		case FID_UA:
			ret = agentHLR_ProUAXosMsg(pxosMsg);
			break;
		default:
			XOS_Trace(MD(FID_HLR,PL_ERR),"agentHLR_XosMsgProc recv msg from unknown Fid(%d) msg",pxosMsg->datasrc.FID);
			ret = XERROR;
			break;
    }

    
    return ret;
}

/**************************************************************************
函 数 名: agentHLR_TimeoutProc
函数功能: 用于向XOS平台注册超时回调接口,暂时不需要进行处理
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 agentHLR_TimeoutProc(t_BACKPARA *tBackPara)
{
    XU32 TimerType = tBackPara->para1;

    switch(TimerType)
    {
        case TIMER_TYPE_AGENT_QRYUSM:
            /*查询控制块超时删除处理*/
            agentHLR_TimerUserInfoPro(tBackPara);
            break;
        case TIMER_TYPE_AGENT_USERCHECK:
            /*用户缓存信息核查定时器处理*/
            agentHLR_UserInfoCheckPro(tBackPara);
            break;
        case TIMER_TYPE_AGENT_UTBIND_SYN:
            /*租赁绑定关系的同步*/
            //agentHLR_UtInfoTimerSynPro(tBackPara);
            break;
        default:
			XOS_Trace(MD(FID_HLR,PL_ERR),"Unknow timer type = %u", TimerType);
            break;

    }


	return XSUCC;
}

XS8 agentHLR_NoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{
	return XSUCC;
}




