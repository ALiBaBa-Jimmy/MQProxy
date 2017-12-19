/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年9月14日
**************************************************************************/
#include "agentuaXos.h"
#include "agentuaCommon.h"
#include "fid_def.h"




XU32 smppUA_FindPeerLinkIDByPeerIp(XS8* peerIp)
{
	tSmppLink *pSmpplink;
	XS32 iCurSize = XOS_listCurSize(gSmppLinkList);
	XS32 iCurIndex;
	XS32 iTemp=0;

	iCurIndex=XOS_listHead(gSmppLinkList);
	while (iTemp < iCurSize) 
	{
		pSmpplink = (tSmppLink *)XOS_listGetElem(gSmppLinkList, iCurIndex);

		if (!XOS_StrCmp(peerIp, pSmpplink->peerMasterIp)) 
		{
			return pSmpplink->index;
		}
		iCurIndex=XOS_listNext(gSmppLinkList, iCurIndex);
		iTemp++;
	}

	return XERROR;
}

/*****************************************************************************
 函 数 名  : agentUA_FindPeerAppHandle
 功能描述  : 查询链路操作
 输入参数  : XU32 linkID   
             XU8 peerType  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 agentUA_FindPeerAppHandle(XU32 linkID, XU8 peerType)
{
	tSmppLink *pSmpplink;
	XS32 iCurSize = XOS_listCurSize(gSmppLinkList);
	XS32 iCurIndex;
	XS32 iTemp=0;

	iCurIndex=XOS_listHead(gSmppLinkList);
	while (iTemp < iCurSize) 
	{
		pSmpplink = (tSmppLink *)XOS_listGetElem(gSmppLinkList, iCurIndex);

		if (/*pSmpplink->peerType == peerType &&*/  pSmpplink->linkStatus  == LINKSTATUS_OK) 
		{
			return iCurIndex;
		}
		iCurIndex=XOS_listNext(gSmppLinkList, iCurIndex);
		iTemp++;
	}

	return XERROR;
}

/*****************************************************************************
 函 数 名  : agentUA_XosMsgHandler
 功能描述  : 将消息发送给hlr
 输入参数  : t_XOSCOMMHEAD* pXosMsg  
             XVOID *Para             
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS8 agentUA_XosMsgHandler(t_XOSCOMMHEAD* pXosMsg, XVOID *Para)
{
	XU8 HlrMsgbuf[MSGBUFFER_MAX_DEF] = {0};
	XS32 MsgLen = 0;
	XS32 appHandle = 0;
	XU32 linkID = 0;
	XU8 peerType = 0;
    t_DATAREQ* ptDataReq      = XNULL;
	t_XOSCOMMHEAD* ptCommHead = XNULL;
	tSmppLink* pSmpplink      = XNULL;
	t_AGENTUA* pAgentUaMsg    = XNULL;

	pAgentUaMsg = (t_AGENTUA*)pXosMsg->message;
	if (pAgentUaMsg == XNULL)
	{
		XOS_PRINT(MD(FID_UA, PL_ERR), "smppUA_XosMsgHandler Receive NULL Message!");
		return XERROR;
	}

	linkID = pAgentUaMsg->linkID;
	peerType = pAgentUaMsg->peerType;
	MsgLen = pAgentUaMsg->msgLenth;
	XOS_MemCpy(HlrMsgbuf, pAgentUaMsg->pData, pAgentUaMsg->msgLenth);
    
    /*再次释放MQTT模块申请的hlrmsg内存块*/
	XOS_MemFree(FID_UA, pAgentUaMsg->pData);

	appHandle = agentUA_FindPeerAppHandle(linkID, peerType);
	if (appHandle == XERROR)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), "error find Link info: linkID = %u, peerType = %u.", linkID, peerType);
		return XERROR;
	}

	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_XosMsgHandler XOS_listGetElem failed");
		return XERROR;
	}

	//将SMPP UA消息提交给NTL模块发送
	ptCommHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_DATAREQ));
	if (XNULLP == ptCommHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), "smppUA_XosMsgHandler MsgMemMalloc is Err!");
		return XERROR;
	}
	ptCommHead->datasrc.FID = FID_UA;
	ptCommHead->datadest.FID = FID_NTL;
	ptCommHead->datadest.PID = XOS_GetLocalPID();
	ptCommHead->datasrc.PID = XOS_GetLocalPID();
	ptCommHead->traceID = pXosMsg->traceID;
	ptCommHead->msgID = eSendData;
	ptCommHead->prio = eNormalMsgPrio;
	agentUA_DataReqFill(ptCommHead, appHandle, MsgLen, HlrMsgbuf);

	if(XSUCC != XOS_MsgSend(ptCommHead))
	{
		XOS_PRINT(MD(FID_UA, PL_ERR),"smppUA_XosMsgHandler send smppua msg to ntl failed.");
		ptDataReq = (t_DATAREQ *)ptCommHead->message;
		XOS_MemFree(FID_UA, ptDataReq->pData);
		XOS_MsgMemFree(FID_UA, ptCommHead);
		return XERROR;
	}
    XOS_Trace(MD(FID_UA, PL_DBG), " Send msg from %u to NTL successfully!", pXosMsg->datasrc.FID);
    return XSUCC;
}
