/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
�� �� ��:
��    ��:
ʱ    ��: 2012��10��17��
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
 Description  : ��������mqttģ�����Ϣ
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
 Description  : ��������uaģ�����Ϣ
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
�� �� ��: agentHLR_InitProc
��������: Э��ջ��ʼ������,��Э��ջ���г�ʼ��,ͬʱע��OAM�ص��ӿ�
��    ��:
�� �� ֵ: XSUCC �ɹ�  XERROR ʧ��
**************************************************************************/
XS8 agentHLR_InitProc(XVOID *pPara1, XVOID *Para2)
{

	XS32 RetValue;

    agentHDB_SubLoadUtBindCallback(agentHLR_InsrtCallback);

    
    /*�û����沿�ֳ�ʼ��*/
    RetValue = agentHLR_UserInfoInit();
    if(XSUCC != RetValue)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), " User info Module init failed!");
        return XERROR;
    }

    /*telnet �����ʼ��*/
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
�� �� ��: agentHLR_XosMsgProc
��������: XOSƽ̨��Ϣ������,���ڴ�������XOSƽ̨����Ϣ
��    ��:
�� �� ֵ:
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
            /*����MQTTserver ������������Ϣ ��Ҫ���͵�hlr/UDC��Ԫ*/
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
�� �� ��: agentHLR_TimeoutProc
��������: ������XOSƽ̨ע�ᳬʱ�ص��ӿ�,��ʱ����Ҫ���д���
��    ��:
�� �� ֵ: XSUCC
**************************************************************************/
XS8 agentHLR_TimeoutProc(t_BACKPARA *tBackPara)
{
    XU32 TimerType = tBackPara->para1;

    switch(TimerType)
    {
        case TIMER_TYPE_AGENT_QRYUSM:
            /*��ѯ���ƿ鳬ʱɾ������*/
            agentHLR_TimerUserInfoPro(tBackPara);
            break;
        case TIMER_TYPE_AGENT_USERCHECK:
            /*�û�������Ϣ�˲鶨ʱ������*/
            agentHLR_UserInfoCheckPro(tBackPara);
            break;
        case TIMER_TYPE_AGENT_UTBIND_SYN:
            /*���ް󶨹�ϵ��ͬ��*/
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




