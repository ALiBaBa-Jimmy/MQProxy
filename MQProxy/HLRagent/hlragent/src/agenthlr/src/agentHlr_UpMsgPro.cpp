/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能: 上行消息处理
时    间: 2012年10月17日
**************************************************************************/

#include "agentHlrCom.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"
#include "agentHlrUtBind.h"


/*****************************************************************************
 Prototype    : agentHLR_ProHlrReqFromMqtt
 Description  : 处理MQTT推送上来的请求消息
 Input        : t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/6/18
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_ProHlrReqFromMqtt(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XS32 ret = XERROR;
    XU8 ucOperateType = pHlrMsg->OpHead.ucOperateTypeId;
    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Enter ucOperateType = %u.", ucOperateType);
    switch(ucOperateType)
    {
        case e_Qry_OperType:

            ret = OperType_ProcQryAllInfoReq(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_UpdateLocation_OperType:
            /*注册更新流程由推送至udc改为直接推送至归属地*/
            //ret = OperType_ProcULReq(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_QryNetWorkId_OperType:

            ret = agentHLR_QryNetIDRequestMsg(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Sag_AuthInfoUpdate:

            //ret = OperType_ProcAuthReq(FID_HLR, pHlrMsg, agentUaMsgLen);
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Sag_AuthInfoDelete:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Delete_OperType:
            /*直接透传给UA-->HLR*/
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_UpdateHome_OperType:
            
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_BusinessChange_OperType:
            /*直接透传至UDC*/
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Smc_Report_SM_Deliviry_Status:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            
            break;
        case e_Smc_AlertRsp_Status:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            
            break;
        case e_Smc_Alert_Service_Centre:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            
            break;
        case e_Sag_PullQryUidbyTel:

            ret = agentHLR_QryUIDRequestMsg(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_auth_SynUtBindInfo:
            
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Auth_UtLeaseSyn:
            ret = agentHLR_ProcUtLeaseMsg(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
		case e_Notify_Insert_Req:
			{
				//直接透传给HLR去处理
				ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
			}
			break;
        case e_Auth_ModifyPidInfo:
            /*向所有的hlr分发*/
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_QryuidVisitNetId:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        default:
            XOS_Trace(MD(FID_HLR,PL_ERR), "Unknow ucOperateType = %u.", ucOperateType);
            return XERROR;
    }
    


    return ret;
}


/*****************************************************************************
 Prototype    : agentHLR_ProHlrReqFromUa
 Description  : 处理UA上来的请求消息
 Input        : t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/6/18
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_ProHlrReqFromUa(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XS32 ret = XERROR;
    XU8 ucOperateType = pHlrMsg->OpHead.ucOperateTypeId;
    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Enter ucOperateType = %u.", ucOperateType);
    switch(ucOperateType)
    {
        case e_Qry_OperType:
            ret = agentHLR_QryMsgProc(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_UpdateLocation_OperType:
            /*更新用户位置信息 请求*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_USER_REGISTER_SUCC_INFO);
            break;
        case e_QryNetWorkId_OperType:

            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_CALLING_REQ_QryNetWork);
            break;
            
        case e_BusinessChange_OperType:
            /*直接订阅*/
            //OperType_SubscribeorNot(FID_HLR, (XCHAR*)pHlrMsg->OpHead.uctopic, MQTT_MSG_ID_SUBSCRI);
            /*业务变更*/
            //ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_USER_BUSINESS_CHANGE);
            break;
        case e_Subscribe_OperType:
            ret = OperType_SubscribeorNot(FID_HLR, (XCHAR*)pHlrMsg->OpHead.uctopic, MQTT_MSG_ID_SUBSCRI);
            break;
        case e_unSubscribe_OperType:
            ret = OperType_SubscribeorNot(FID_HLR, (XCHAR*)pHlrMsg->OpHead.uctopic, MQTT_MSG_ID_unSUBSCRI);
            break;
        case e_Smc_Report_SM_Deliviry_Status:
            /*推送至注册地*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_Smc_AlertRsp_Status:
            /*推送至注册地*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_Smc_Alert_Service_Centre:
            /* 此字段里封装的是topic是开户地的网络号，直接推送*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_ReadyforSMAlertMsg:
            /* 此字段里封装的是topic是开户地的网络号，直接推送*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_Sag_AuthInfoUpdate:
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_Sag_AuthInfoDelete:
            //agentHLR_UpdateAuthMsgProc(FID_HLR, pHlrMsg, agentUaMsgLen);
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_UpdateHome_OperType:
            /*对于更新消息来说，可能还需要更新agent用户的缓存信息*/
            ret = agentHLR_UpdateHomeMsgProc(FID_HLR, pHlrMsg, agentUaMsgLen);
            /*从拜访地发送过来的更新直接推送至归属地*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_Delete_OperType:
            /*从归属地发送过来的delete Message直接推送至老的拜访地*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            
            break;
        case e_Sag_PullQryUidbyTel:
            /*暂时借用 MQTT_CALLING_REQ_QryNetWork 推送至UDC*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_CALLING_REQ_QryNetWork);
            break;
        case e_Auth_ModifyPidInfo:
            /*向所有的hlr分发*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_DELIVERY_2_HLR);
            break;
        case e_Auth_UtLeaseSyn:
            /*向udc同步租赁信息*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, MQTT_CALLING_REQ_QryNetWork);
            break;
		case e_Notify_Insert_Req:
			//直接将消息推送给需要下发insert请求的网元
			ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
			break;
        case e_QryuidVisitNetId:
            /*呼叫 查询被叫网络号*/
            ret = agentHLR_QryCalledTelMsgProc(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
      default:
            XOS_Trace(MD(FID_HLR,PL_ERR), "Unknow ucOperateType = %u.", ucOperateType);
            return XERROR;
    }
    


    
    return ret;
}



XS32 OperType_ProcQryAllInfoReq(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    /*查询用户详细信息*/
    /*填充模块间通信结构头*/
    AgentUaMsg.msgLenth = agentUaMsgLen;
    AgentUaMsg.pData = (XCHAR *)XOS_MemMalloc(FID_HLR, agentUaMsgLen);
    if(NULL == AgentUaMsg.pData)
    {
        XOS_Trace(MD(fid,PL_ERR), "XOS_MemMalloc failed.");
        return ERROR;
    }
    XOS_MemCpy(AgentUaMsg.pData, pHlrMsg, agentUaMsgLen);
    

    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_UA);
    if(XERROR == ret)
    {
        
        XOS_MemFree(fid, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(fid,PL_ERR), "ret = %u.", ret);
    }
    return ret;

}





XVOID agentHLR_PrintULInfo(XU32 fid, SUpdateUserInfoReq *pNew, TXdbHdbDyn *pOld)
{
    XOS_Trace(MD(fid,PL_DBG), "========New info=======");
    XOS_Trace(MD(fid,PL_DBG), "RegType = %u", pNew->RegType);
    XOS_Trace(MD(fid,PL_DBG), "networkid = %s", pNew->cNetworkID);
    XOS_Trace(MD(fid,PL_DBG), "vssid = %02X%02X%02X%02X", pNew->VssId[0],pNew->VssId[1],pNew->VssId[2],pNew->VssId[3]);
    XOS_Trace(MD(fid,PL_DBG), "sagip = %u", pNew->sagIp);
    XOS_Trace(MD(fid,PL_DBG), "public ip = %u", pNew->CamtalkInfo.iCssPublicIp);
    XOS_Trace(MD(fid,PL_DBG), "private ip = %u", pNew->CamtalkInfo.iCssPrivateIp);
    XOS_Trace(MD(fid,PL_DBG), "cssid = %02X%02X%02X%02X", pNew->CamtalkInfo.cssId[0], pNew->CamtalkInfo.cssId[1], pNew->CamtalkInfo.cssId[2], pNew->CamtalkInfo.cssId[3]);


    XOS_Trace(MD(fid,PL_DBG), "========Old info=======");

}

/*****************************************************************************
 Prototype    : agentHLR_QryUIDRequestMsg
 Description  : 根据电话号码查询uid消息处理
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/21
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_QryUIDRequestMsg(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XU16 bufLen = 0;
    XS32 ret = XSUCC;
    
    t_Sj3_Oper_Rsp ossMsgReq = {0};
    XU16 len = 0;
    XU8 buf[512] = {0};

    XU8 telStr[33] = {0};
    XU8 tel[LENGTH_OF_TEL] = {0};
    XU8 uid[LENGTH_OF_UID] = {0};


    
    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    

    /*解码 消息*/
    ret = agentHLR_DecodeTLV(pHlrMsg->usBuf, bufLen, e_TELNO_TAG, tel);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Decode tel Err: bufLen=%u.", bufLen);
        return XERROR;
    }

    

    DB_StrToHex(tel, telStr, LENGTH_OF_TEL);
    
    XOS_Trace(MD(fid,PL_DBG), "Decode : telStr=%s", telStr);

    /*根据电话号码查询UID*/
    ret = agentDB_QryuidbyTel(fid, uid, tel);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid, PL_ERR), " qry UID BY tel failed");
        return XERROR;
    }
 
    XOS_Trace(MD(fid,PL_DBG), "Qry Result: uid=%08X ", XOS_NtoHl(*(XU32*)uid));

    //构造OSS消息内容
    len += agentHLR_EncodeTLV(buf, e_UID_TAG, uid, LENGTH_OF_UID);
    len += agentHLR_EncodeTLV(buf+len, e_TELNO_TAG, tel, LENGTH_OF_TEL);
    ENCODEOSSRSP(ossMsgReq, e_Sag_PullQryUidbyTel, len, pHlrMsg->Ph.usDlgId);
    
    XOS_MemCpy(ossMsgReq.usBuf, buf, len);
	XOS_MemCpy(ossMsgReq.OpHead.uctopic, pHlrMsg->OpHead.uctopic, LENGTH_OF_NETWORK_ID);

    /*向udc推送查询networkid 的Rsp*/
    ret = OperType_Push(FID_HLR, &ossMsgReq, len+LENGTH_HEAD+LENGTH_TAIL, (XCHAR*)pHlrMsg->OpHead.uctopic);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR, PL_ERR), "PUSH TO HLR Qry uid error!ret = %u,hlr=%s", ret, pHlrMsg->OpHead.uctopic);
        
        return XERROR;
    }
    
    XOS_Trace(MD(fid, PL_DBG),"Exit ");
    return XSUCC;
}

#define   ____ESMC__


/*****************************************************************************
 Prototype    : OperType_ProEsmcReport
 Description  : 通知消息的处理：主要同步数据
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/8/14
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 OperType_ProEsmcReport(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XS32 ret;
    TXdbHdbDyn tDynInfo = {0};
    XU16 buflen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    tRep2Agent ReportInfo = {0};
    XU8 LocalNetwork[LENGTH_OF_NETWORK_ID] = {0};
    

    if(XERROR == agentHLR_DecodeSmcReport(pHlrMsg->usBuf, buflen, &ReportInfo))
    {
        /* 信元必带*/
        XOS_Trace(MD(fid,PL_ERR), "ReportInfo decode error !");

        return XERROR;
    }
    XOS_Trace(MD(fid,PL_DBG), "uid = %02X%02X%02X%02X, status=%02X",
                ReportInfo.uid[0],ReportInfo.uid[1],ReportInfo.uid[2],ReportInfo.uid[3],
                ReportInfo.hsStatus);
    /*获取动态信息*/
    ret = agentDB_QryDynInfobyUID(fid, ReportInfo.uid, &tDynInfo);
    if(XSUCC != ret)
    {

        XOS_Trace(MD(fid,PL_ERR), " Qry dyninfo failed!");
        return XERROR;
    }
    XOS_Trace(MD(fid,PL_DBG), "status=%02X", tDynInfo.hsStatus);
    
    if(tDynInfo.hsStatus != ReportInfo.hsStatus)
    {
        tDynInfo.hsStatus = ReportInfo.hsStatus;

        /*更新动态信息*/
        ret = agentDB_UpdateDynInfo(fid, ReportInfo.uid, &tDynInfo);
        if(XSUCC != ret)
        {
            XOS_Trace(MD(fid,PL_ERR), " update dyninfo failed!,uid = %02X%02X%02X%02X", 
                ReportInfo.uid[0],ReportInfo.uid[1],ReportInfo.uid[2],ReportInfo.uid[3]);
            return XERROR;
        }
        else
        {
            if(0 != XOS_MemCmp(ReportInfo.cNetworkID, tDynInfo.cNetworkID, LENGTH_OF_NETWORK_ID))
            {
                /*此用户属于异地拜访\注册，需要向注册地推送用户信息更新*/
                PushUserInfo(ReportInfo.uid, tDynInfo.cNetworkID);
            }   
            if(XSUCC == agentDB_QryLocalNetWorkIdbyUID(fid, ReportInfo.uid, LocalNetwork))
            {
                if(0 != XOS_MemCmp(ReportInfo.cNetworkID, LocalNetwork, LENGTH_OF_NETWORK_ID))
                {
                    /*此用户需要向开户地同步status字段*/
                    Push2LocalHlrUpdateStatus(&ReportInfo, LocalNetwork);
                }

            }
        }
        
        
    }
    
    return XSUCC;

}



/*****************************************************************************
 Prototype    : PushHomehlr
 Description  : 向归属地推送该uid的详细信息
 Input        : XU8 *pTel     
                XU8 *pNewNet  
                XU8 * pUid    
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/1
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XS32 PushUserInfo( XU8 * pUid, XU8* topic)
{
    XU8 buf[2048] = {0};
    XU16 len = 0;
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret;
    
    t_Sj3_Oper_Rsp * hlrmsg = (t_Sj3_Oper_Rsp *)buf;




    hlrmsg->Ph.ucPackType = SJ_REQ;
    hlrmsg->Ph.ossId = 0;

    /*由agent发起的请求相信信息，此字段需要填0*/
    hlrmsg->Ph.usDlgId = 0;

    
    hlrmsg->OpHead.ucOperateObjId = e_OperObj;
    hlrmsg->OpHead.ucOperateTypeId = e_Qry_OperType;
    hlrmsg->OpHead.ucIsSucc = XSUCC;
    hlrmsg->OpHead.ucIsEnd = 0x01;
    hlrmsg->OpHead.ucPackId = 0;
    XOS_MemCpy( hlrmsg->OpHead.uctopic, topic, XOS_StrLen(topic));
    /*封装UID 信元*/
    len += agentHLR_EncodeTLV(hlrmsg->usBuf, e_UID_TAG, pUid, 4);
    
    hlrmsg->OpHead.usLen = XOS_HtoNs(len);






    AgentUaMsg.msgLenth = len+LENGTH_HEAD+LENGTH_TAIL;
    AgentUaMsg.pData = (XCHAR *)XOS_MemMalloc(FID_HLR, AgentUaMsg.msgLenth);
    if(NULL == AgentUaMsg.pData)
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "XOS_MemMalloc failed.");
        return ERROR;
    }
    XOS_MemCpy(AgentUaMsg.pData, hlrmsg, AgentUaMsg.msgLenth);
    

    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_UA);
    if(XERROR == ret)
    {
        
        XOS_MemFree(FID_HLR, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(FID_HLR,PL_ERR), "ret = %u.", ret);
    }

    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Exit topic = %s, len = %u.", topic, len);
    return ret;
    
}

XS32 Push2LocalHlrUpdateStatus(tRep2Agent* pInfo, XU8 * pTopic)
{
    XU8 buf[2048] = {0};
    XU16 len = 0;
    XU8 uidstr[9] = {0};

    XOS_Trace(MD(FID_HLR,PL_DBG), "Enter ");

    t_Sj3_Oper_Rsp * hlrmsg = (t_Sj3_Oper_Rsp *)buf;
    
    hlrmsg->Ph.ucPackType = SJ_REQ;
    hlrmsg->Ph.ossId = 0;
    hlrmsg->Ph.usDlgId = 0;

    
    hlrmsg->OpHead.ucOperateObjId = e_OperObj;
    hlrmsg->OpHead.ucOperateTypeId = e_Smc_Report_SM_Deliviry_Status;
    hlrmsg->OpHead.ucIsSucc = XSUCC;
    hlrmsg->OpHead.ucIsEnd = 0x01;
    hlrmsg->OpHead.ucPackId = 0;

    
    XOS_MemCpy( hlrmsg->OpHead.uctopic, pTopic, XOS_StrLen(pTopic));
    /*封装 信元*/
    len += agentHLR_EncodeTLV(hlrmsg->usBuf, e_AGENT_Report, pInfo, sizeof(tRep2Agent));


    
    XS32 ret = OperType_Push(FID_HLR, hlrmsg, len+LENGTH_HEAD+LENGTH_TAIL,(XCHAR *)pTopic);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "PUSH TO mqtt error!ret = %u", ret);
    }
    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Exit : len = %u", len);
    return ret;
    
}

/*****************************************************************************
 Prototype    : agentHLR_UpdateHomeMsgProc
 Description  : 更新消息需要更新agent的缓存
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UpdateHomeMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    TXdbHdbDyn DynInfo = {0};
    XU16 bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    XU16 DynLen = sizeof(tUpdateDyn);

    if(bufLen != DynLen+3)/*tag+len=3*/
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "msgLen: %u != Dyninfo : %u", bufLen, DynLen);
        return XERROR;
    }
    XOS_MemCpy(&DynInfo, pHlrMsg->usBuf+3, sizeof(TXdbHdbDyn));

    
    tContext* pstContext = UserInfo_Qrybyuid_Ext(DynInfo.uid);
    if(XNULL != pstContext)
    {
    
        /*更新agent缓存的动态信息*/
        #if 1

        pstContext->tHashDyn.hsStatus = DynInfo.hsStatus;
        pstContext->tHashDyn.hsVersion = DynInfo.hsVersion;
        XOS_MemCpy(pstContext->tHashDyn.bsID, DynInfo.bsId, LENGTH_OF_BSID);
        XOS_MemCpy(pstContext->tHashDyn.bscID, DynInfo.bscId, LENGTH_OF_BSCID);
        XOS_MemCpy(pstContext->tHashDyn.laiID, DynInfo.laiId, LENGTH_OF_LAIID);
        XOS_MemCpy(pstContext->tHashDyn.vssID, DynInfo.vssId, LENGTH_OF_VSSID);

        pstContext->tHashDyn.xwipv4 = DynInfo.xwipv4;
        pstContext->tHashDyn.xwipport = DynInfo.xwipport;
        XOS_MemCpy(pstContext->tHashDyn.pid, DynInfo.pid, LENGTH_OF_PID);
        XOS_MemCpy(pstContext->tHashDyn.currRegDate, DynInfo.currRegDate, LENGTH_OF_DATE);
        XOS_MemCpy(pstContext->tHashDyn.prevRegDate, DynInfo.prevRegDate, LENGTH_OF_DATE);

        XOS_MemCpy(&pstContext->tHashDyn.voiceAnchor, &DynInfo.voiceAnchor, sizeof(TXdbScuVoiceAnchor));
        pstContext->tHashDyn.pidPort = DynInfo.pidPort;
        pstContext->tHashDyn.sagIpv4 = DynInfo.sagIpv4;
        XOS_MemCpy(&pstContext->tHashDyn.hashCamtalkDyn, &DynInfo.camtalkDynInfo, sizeof(SHlrCamtalkDynInfo));
        
        XOS_MemCpy(pstContext->tHashDyn.cNetworkID, DynInfo.cNetworkID, LENGTH_OF_NETWORK_ID);
        XOS_MemCpy(pstContext->tHashDyn.authNet, DynInfo.authNet, LENGTH_OF_NETWORK_ID);
        
        pstContext->tHashDyn.sagIp_voice = DynInfo.sagIp_voice;
        pstContext->tHashDyn.sagIp_data = DynInfo.sagIp_data;
        

        
        pstContext->updatetime = GetPresentTime();
        #endif
    }
    
    
    return XSUCC;
}


/*****************************************************************************
 Prototype    : agentHLR_UpdateAuthMsgProc
 Description  : 更新消息需要更新agent的缓存
 Input        : XU32 fid                 
                t_Sj3_Oper_Rsp *pHlrMsg  
                XU32 agentUaMsgLen       
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/9/28
    Author       : wangdanfeng
    Modification : Created function

*****************************************************************************/
XS32 agentHLR_UpdateAuthMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    
    tAuthInfo authInfo = {0};
    XU16 bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);
    XU16 tmpLen = sizeof(tAuthInfo);

    if(bufLen != tmpLen+3)/*tag+len=3*/
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "msgLen: %u != Dyninfo : %u", bufLen, tmpLen);
        return XERROR;
    }
   // XOS_MemCpy(&authInfo, pHlrMsg->usBuf+3, tmpLen);
    if(XSUCC != agentHLR_DecodeTLV(pHlrMsg->usBuf, bufLen, e_AGENT_AUTHINFO, &authInfo))
    {
        XOS_Trace(MD(FID_HLR,PL_ERR), "Decode e_AGENT_AUTHINFO Err");

        return XERROR;
    }
    
    tContext* pstContext = UserInfo_Qrybyuid_Ext(authInfo.uid);
    if(XNULL != pstContext)
    {
    
        /*更新agent缓存的动态信息*/


        pstContext->tHashDyn.hsStatus = authInfo.hsStatus;
       
        XOS_MemCpy(pstContext->tHashDyn.pid, authInfo.pid, LENGTH_OF_PID);
        pstContext->tHashDyn.pidPort = authInfo.pidPort;
        XOS_MemCpy(pstContext->tHashDyn.authNet, authInfo.authNet, LENGTH_OF_NETWORK_ID);
                
        pstContext->updatetime = GetPresentTime();
      
    }
    
    
    return XSUCC;
}
#define   ___HLR_QRY_CALLED_USER___
/*查询被叫*/
extern XS32 agentHLR_QryUserNetIdbyDNS(tUSM *pQryUSM, t_Sj3_Oper_Rsp *pMsgOper);


XS32 agentHLR_QryCalledTelMsgProc(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{

    XU16 bufLen = 0;
    XS32 ret;
    tUSM tQryInfo = {0};
    tContext tUserInfo = {0};
    


    XOS_Trace(MD(fid, PL_DBG),"Enter ");
    
    bufLen = XOS_NtoHs(pHlrMsg->OpHead.usLen);



    ret = ProcessDecodeQryUserAllInfoReq((XU8*)&tQryInfo, pHlrMsg->usBuf, bufLen);
    if(XSUCC != ret)
    {
        XOS_Trace(MD(fid,PL_ERR), "Qryinfo msg decode failed.");
        return XERROR;
    }
    
    tQryInfo.operatorTyp = e_QryuidVisitNetId;


    

    /*向DNS/UDC请求uid的网络路由*/
    return agentHLR_QryUserNetIdbyDNS(&tQryInfo, pHlrMsg);


}
