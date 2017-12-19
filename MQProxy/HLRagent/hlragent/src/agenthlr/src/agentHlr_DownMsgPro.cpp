/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年10月17日
**************************************************************************/

#include "agentHlrCom.h"
#include "agentHlrUserMemInfo.h"
#include "agentHlrUserInfoQryProc.h"

XS32 agentHLR_DBTest(XU32 UID)
{
  //  TXdbHashDyn dynInfo = {0};
//    XU8 uid[4];
  //  XU8 uidStr[9] = {0};
  //  XU8 pidstr[9] = {0};
 //   XU8 bsidstr[9] = {0};
    //XU32 UID = 1644467449;
//    XS32 ret;




    XOS_Trace(MD(FID_HLR, PL_DBG), " succefully Exit. ");
    return XSUCC;
}



/*处理MQTT推送下来的响应消息，*/
XS32 agentHLR_ProHlrRspFromMqtt(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    t_AGENTUA AgentUaMsg = {0};
    XS32 ret = XERROR;
    XU8 ucOperateType = pHlrMsg->OpHead.ucOperateTypeId;
    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Enter ucOperateType = %u.", ucOperateType);
    switch(ucOperateType)
    {
        case e_Qry_OperType:
            //ret = OperType_QryAllInfoRsptoUA(FID_HLR, pHlrMsg, agentUaMsgLen);   
            ret = agentHLR_QryUserInfoRspMsgPro(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_UpdateLocation_OperType:
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);            
            break;
        case e_QryNetWorkId_OperType:
            //ret = OperType_ProcULRsptoUA(FID_HLR, pHlrMsg, agentUaMsgLen); 
            ret = agentHLR_QryNetRspMsgPro(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_Sag_PullQryUidbyTel:
            /*根据电话号码 查询UID响应 直接透传给hlr*/
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);            
            break;
        case e_UpdateHome_OperType:
            /*直接透传给hlr*/
            ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);            
            break;
        case e_QryuidVisitNetId:
             ret = OperType_TransmittoUA(FID_HLR, pHlrMsg, agentUaMsgLen);
             break;   
        default:
            
            return XERROR;
    }
    


    return ret;
}




/*处理UA上来的响应消息，*/
XS32 agentHLR_ProHlrRspFromUa(t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    XS32 ret = XERROR;
    XU8 ucOperateType = pHlrMsg->OpHead.ucOperateTypeId;
    
    XOS_Trace(MD(FID_HLR,PL_DBG), "Enter ucOperateType = %u.", ucOperateType);
    switch(ucOperateType)
    {
        case e_Qry_OperType:
            /*推送 查询所有信息 的响应*/
            ret = OperType_QryAllInfoRspPush(FID_HLR, pHlrMsg, agentUaMsgLen);
            break;
        case e_UpdateHome_OperType:
            /*推送  的响应*/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_BusinessChange_OperType:
            /**/
            ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
            break;
        case e_QryuidVisitNetId:
             ret = OperType_Push(FID_HLR, pHlrMsg, agentUaMsgLen, (XCHAR*)pHlrMsg->OpHead.uctopic);
             break;   

        default:
            
            return XERROR;
    }
    


    
    return ret;
}


#define   __NEXT__________

XS32 OperType_QryAllInfoRsptoUA(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    return OperType_TransmittoUA(fid, pHlrMsg, agentUaMsgLen);

}



XS32 OperType_QryAllInfoRspPush(XU32 fid, t_Sj3_Oper_Rsp *pHlrMsg, XU32 agentUaMsgLen)
{
    t_AGENTUA AgentUaMsg = {0};
    XU8  tel[LENGTH_OF_TEL] = {0};
    XS32 ret;
    XU16 buflen = XOS_NtoHs(pHlrMsg->OpHead.usLen);


    AgentUaMsg.msgLenth = agentUaMsgLen;
    /*获取推送的主题*/


    
    XOS_MemCpy(AgentUaMsg.topic, pHlrMsg->OpHead.uctopic, XOS_StrLen(pHlrMsg->OpHead.uctopic));
    //XOS_MemCpy(AgentUaMsg.topic, "13120199852", XOS_StrLen("13120199852"));
    AgentUaMsg.msgID = MQTT_MSG_ID_PUSH;
    AgentUaMsg.pData = (XCHAR *)XOS_MemMalloc(fid, agentUaMsgLen);
    if(NULL == AgentUaMsg.pData)
    {
        XOS_Trace(MD(fid,PL_ERR), "XOS_MemMalloc failed.");
        return ERROR;
    }
    XOS_MemCpy(AgentUaMsg.pData, pHlrMsg, agentUaMsgLen);



    ret = agentHLR_SendHlrMsg(&AgentUaMsg, FID_MQTT);
    if(XERROR == ret)
    {
        
        XOS_MemFree(fid, AgentUaMsg.pData);
        AgentUaMsg.pData = NULL;
        XOS_Trace(MD(fid,PL_ERR), "ret = %u.", ret);
    }
    return ret;
}

