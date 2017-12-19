#include "agentuaOam.h"
#include "fid_def.h"
#include "common_msgdef.h"
#include "agentProtocol.h"

XOS_HLIST gSmppLinkList;


XVOID agentUA_LinkInit(tPeSmppCfgOamTable* pPeCfgInfo, tSmppLink* pSmppLinkRecord, XS32 FMTpMsg)
{
	pSmppLinkRecord->index = pPeCfgInfo->index;
	pSmppLinkRecord->linkType = eTCPServer;

	//XOS_StrCpy(pSmppLinkRecord->smcMasterIp, pPeCfgInfo->smcIp);
	XOS_MemCpy(pSmppLinkRecord->smcMasterIp, pPeCfgInfo->smcIp, MAX_HOST_DEF);
	pSmppLinkRecord->smcPort = pPeCfgInfo->smcPort;
	//XOS_StrCpy(pSmppLinkRecord->peerMasterIp, pPeCfgInfo->peerIp);
	XOS_MemCpy(pSmppLinkRecord->peerMasterIp, pPeCfgInfo->peerIp, MAX_HOST_DEF);
	pSmppLinkRecord->peerPort = pPeCfgInfo->peerPort;

	//XOS_StrCpy(pSmppLinkRecord->linkDesc, pPeCfgInfo->linkDesc);
	XOS_MemCpy(pSmppLinkRecord->linkDesc, pPeCfgInfo->linkDesc,MAX_LINK_DESC_DEF);
	pSmppLinkRecord->linkStatus  = LINKSTATUS_ERROR;
	pSmppLinkRecord->hTimerHandle = NULL;
	pSmppLinkRecord->linkHandle = NULL;
	pSmppLinkRecord->usRepeat = 0;
	pSmppLinkRecord->isFmtMsg = FMTpMsg;
}




XS32 agentUA_AddLinkCfgRecord(AGT_OAM_CFG_REQ_T *pMsg, XS32 FMTpMsg)
{
	XS32 ret, i;
	XS32 iListIndex = 0;
	t_XOSCOMMHEAD *pMsgHead;
	t_LINKINIT tLinkInit;
	tSmppLink tSmppLinkRecord = {0};
	tPeSmppCfgOamTable* pPeSmppCfgInfo = XNULL;


	pPeSmppCfgInfo = (tPeSmppCfgOamTable*)pMsg->pData;
	for ( i = 0;i < 1;i ++)
	{
    		tSmppLinkRecord.peerType = PEER_HLR;
    		agentUA_LinkInit(pPeSmppCfgInfo, &tSmppLinkRecord,FMTpMsg);
    		if (i==0)
    		{
    		    //如果为对端类型为PEER_HLR，则启一个服务器，启一个客户端
    			tSmppLinkRecord.linkType = pPeSmppCfgInfo->linkType;
    		}
    		else
    		{
    			tSmppLinkRecord.linkType = eTCPServer;
    		}
    		iListIndex = XOS_listAddTail(gSmppLinkList, &tSmppLinkRecord);
    		if (iListIndex == XERROR)
    		{

    			//XOS_Trace(XNULL,MD(FID_UA, PL_ERR), "smppUA_AddPeSmcRoamCfgRecord XOS_listAddTail is Err!");
    			return XERROR;
    		}

			/*向NTL模块发送LinkInit消息*/
			pMsgHead = XOS_MsgMemMalloc(FID_UA, sizeof(t_LINKINIT));
			if (XNULLP == pMsgHead)
			{

				return XERROR;
			}

			pMsgHead->datasrc.FID = FID_UA;
			pMsgHead->datadest.FID = FID_NTL;
			pMsgHead->msgID = eLinkInit;
			pMsgHead->prio = eNormalMsgPrio;

			tLinkInit.appHandle = (HAPPUSER)iListIndex;
			
			tLinkInit.linkType = (e_LINKTYPE)tSmppLinkRecord.linkType;
			XOS_MemCpy(pMsgHead->message, &tLinkInit, sizeof(t_LINKINIT));

			ret = XOS_MsgSend(pMsgHead);
			if (XERROR == ret)
			{
				/*释放掉内存空间*/
				XOS_MemFree(FID_UA, pMsgHead);


				//XOS_Trace(XNULL,MD(FID_UA,PL_ERR), "smppUA_AddPeSmcRoamCfgRecord XOS_MsgSend is Err!");
				return XERROR;
			}
            memset(&tSmppLinkRecord, 0, sizeof(tSmppLinkRecord));
		}

	
	
	return XSUCC;
}

XS8 agentUA_ConvertTableIDToPeerType(XU32 tableID)
{
	XS8 peerType; 
	switch (tableID)
	{
		case MML_UA_LINK_TABLE_ID:
			peerType = PEER_HLR;
			break;
		case MML_UA_LINK_TOUDC_TABLE_ID:
			peerType = PEER_UDC;
			break;
		default:
			peerType = -1;
			break;
	}
	return peerType;
}

XS32 agentUA_GetLinkHandle(XS32 linkID, XS8 peerType, XS32* iListIndex)
{
	tSmppLink *pSmpplink;
	XS32 iCurSize = XOS_listCurSize(gSmppLinkList);
	XS32 iCurIndex;
	XS32 iTemp = 0;

	iCurIndex=XOS_listHead(gSmppLinkList);
	while (iTemp < iCurSize) 
	{
		pSmpplink = (tSmppLink *)XOS_listGetElem(gSmppLinkList, iCurIndex);

		if ((pSmpplink->index == linkID)&&(pSmpplink->peerType == peerType)) 
		{
			*iListIndex = iCurIndex;
			return XSUCC;
		}
		iCurIndex=XOS_listNext(gSmppLinkList, iCurIndex);
		iTemp++;
	}

	XOS_Trace(MD(FID_UA,PL_ERR),"smppUA_PeCfgIndexMatch Can't Find The smpp link record!");
	*iListIndex = -1;
	return XERROR;
}

XS32 agentUA_DelPeCfgRecord(AGT_OAM_CFG_REQ_T *pMsg)
{
	XS32 ret, i;
	XS32 iListIndex = 0;
	t_XOSCOMMHEAD *pMsgHead;
	tSmppLink* pSmppLink = XNULL;
	t_LINKRELEASE tLinkRelease = {0};
	XU32 linkID = 0;
	XS8 peerType = 0;



	linkID = *(XU32 *)pMsg->pData ;
	peerType = agentUA_ConvertTableIDToPeerType(pMsg->uiTableId);
	if (peerType == XERROR)
	{
		XOS_Trace(MD(FID_UA,PL_ERR),"smppUA_DelPeCfgRecord tableID is wrong!");
		return XERROR;
	}

	//if (peerType == PEER_HLR || peerType == PEER_UDC)
	{
		for (i = 0;i < 1;i ++)
		{
			ret = agentUA_GetLinkHandle(linkID, peerType, &iListIndex);
			if (XERROR == ret)
			{

				XOS_Trace(MD(FID_UA,PL_ERR), "smppUA_DelPeCfgRecord smppUA_PeCfgIndexMatch is Err!");
				return XERROR;
			}
			pSmppLink = (tSmppLink *)XOS_listGetElem(gSmppLinkList, iListIndex);

			/*向NTL发送LinkRelease 请求消息*/
			pMsgHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA,sizeof(t_LINKRELEASE));
			if (XNULLP == pMsgHead)
			{


				XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_DelPeCfgRecord XOS_MsgMemMalloc is Err!");
				return XERROR;
			}

			pMsgHead->datasrc.FID = FID_UA;
			pMsgHead->datadest.FID = FID_NTL;
			pMsgHead->msgID = eLinkRelease;
			pMsgHead->prio = eNormalMsgPrio;
			tLinkRelease.linkHandle = pSmppLink->linkHandle;
			XOS_MemCpy(pMsgHead->message, &tLinkRelease, sizeof(t_LINKRELEASE));

			ret=XOS_MsgSend(pMsgHead);
			if (XERROR == ret)
			{
				/*释放内存*/
				XOS_MemFree(FID_UA, pMsgHead);
				XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_DelPeCfgRecord XOS_MsgSend is Err!");
				return XERROR;
			}

			ret = XOS_listDelete(gSmppLinkList, iListIndex);
			if(XFALSE == ret)
			{
				XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_DelPeCfgRecord  XOS_listDelete is Err!");
				return XERROR;
			}
		}
	}
	



	return XSUCC;
}

XS32 agentUA_LinkHandler(AGT_OAM_CFG_REQ_T * pRecvMsg)
{
	XS32 ret;
	tResponsePeCfg pResponsePeCfg[MAX_SMPP_LINK_COUNT] = {0};
	AGT_OAM_CFG_RSP_T oamCfgRsp = {0};
	AGT_OAM_CFG_RSP_T* pOamCfgRsp = XNULL;
	
	

	switch(pRecvMsg->uiOperType)
	{
    	case OAM_CFG_SYNC:
    	case OAM_CFG_ADD:
    		ret = agentUA_AddLinkCfgRecord(pRecvMsg, 0);
    		break;
    	case OAM_CFG_DEL:
    		ret = agentUA_DelPeCfgRecord(pRecvMsg);
    		break;

    	default:
    		XOS_Trace(MD(FID_UA,PL_ERR), "Unknown uiOperType = %u.", pRecvMsg->uiOperType);
    		return XERROR;
	}
    XOS_MemFree(FID_UA, pRecvMsg->pData);
	return ret;
}


