/**************************************************************************
Copyright (C) ,2011-2012, Shenzhen Xinwei. Co.,Ltd
文 件 名:
功    能:
时    间: 2012年9月13日
**************************************************************************/
#include "agentProtocol.h"
#include "agentuaXos.h"
#include "agentuaCommon.h"
#include "xosenc.h"
#include "agentuaOam.h"
#include "fid_def.h"
#include "smu_sj3_type.h"

XOS_HLIST smppUA_htMsgBufferList;

XS32 agentua_EncodeTcpe(XU8* pData, XU16 dataLen)
{


    pData[dataLen - 2] = 0X7E;
    pData[dataLen - 1] = 0X0D;

    pData[0] = 0X7E;
    pData[1] = 0XA5;
    *(XU16*)(pData+2) = XOS_HtoNs(dataLen - 4);

    return XSUCC;
}

XS32 Fuction64To32(XS64 appTempHandle)
{
	XU8 tempHandle[XS64_SIZE] = {0};
	XS64 tempInteger = appTempHandle;
	XS32 appHandle = 0;

	memcpy((char*)tempHandle, (char*)&tempInteger, XS64_SIZE);
	appHandle = *(XS32*)tempHandle;
	return appHandle;
}

XS32 agentUA_HeartSend(XU32 fid, XS32 appHandle, XU32 msgid)
{
    t_XOSCOMMHEAD *pMsgHead;
    t_Sj3_Oper_Rsp tHeartBeat = {0};
    t_DATAREQ *ptDataReq;


    //发送heart消息
	pMsgHead=XOS_MsgMemMalloc(fid, sizeof(t_DATAREQ));
	if (XNULLP == pMsgHead)
	{
		XOS_Trace(MD(fid,PL_ERR), "XOS_MsgMemMalloc NULL!");
		return XERROR;
	}
	pMsgHead->datasrc.FID = fid;
	pMsgHead->datadest.FID = FID_NTL;
	pMsgHead->msgID = eSendData;
	pMsgHead->prio = eNormalMsgPrio;


    tHeartBeat.Ph.ucPackType = SJ_REQ;

    tHeartBeat.OpHead.ucOperateObjId = e_HandShake;

    agentUA_DataReqFill(pMsgHead, appHandle, LENGTH_HEAD+LENGTH_TAIL, (XU8*)&tHeartBeat);

	XU32 ret = XOS_MsgSend(pMsgHead);
	if (XERROR == ret)
	{
		/*释放掉内存空间*/
		ptDataReq = (t_DATAREQ *)(pMsgHead->message);
		XOS_MemFree(fid, ptDataReq->pData);
		XOS_MemFree(fid, pMsgHead);
		XOS_Trace(MD(fid,PL_ERR), "TcpClient MsgSend heart is Err!");
		
	}
	return ret;
}
XS32 agentUA_HeartSendReq(XU32 fid, XS32 appHandle)
{

    return agentUA_HeartSend(fid, appHandle, E_AGENTUA_HEART_REQ);

}
XS32 agentUA_HeartSendRsp(XU32 fid, XS32 appHandle)
{

    return agentUA_HeartSend(fid, appHandle, E_AGENTUA_HEART_RSP);

}

XVOID agentUA_MsgBufferMatch(HLINKHANDLE linkHandle, XS32 *iMsgBufferIndex)
{
	XS32 iCurSize = 0;
	XS32 iCurIndex = 0;
	XS32 iCount = 0;
	MSG_BUFFER_T *ptMsgBuffer;

	iCurSize = XOS_listCurSize(smppUA_htMsgBufferList);
	if (0 == iCurSize)
	{
		XOS_Trace(MD(FID_UA, PL_INFO), "the MsgBufferList is NULL!");
		*iMsgBufferIndex = -1;
		return ;
	}

	iCurIndex = XOS_listHead(smppUA_htMsgBufferList);
	while (iCount < iCurSize)
	{
		ptMsgBuffer = (MSG_BUFFER_T *)XOS_listGetElem(smppUA_htMsgBufferList, iCurIndex);
		if (linkHandle == ptMsgBuffer->linkHandle)
		{
			*iMsgBufferIndex = iCurIndex;
			break;
		}
		iCurIndex=XOS_listNext(smppUA_htMsgBufferList,iCurIndex);
		iCount++;
	}

	if (iCount == iCurSize)
	{
		*iMsgBufferIndex = -1;
	}
}
/*****************************************************************************
 Prototype    : agentUA_DataReqFill
 Description  : 封装tcpe消息到xos头
 Input        : t_XOSCOMMHEAD *pMsgHead  
                XS32 appHandle           
                XU32 uiLength            
                XS8 *pData               
 Output       : None
 Return Value : 
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2015/7/7
    Author       : 处理MQTT推送上来的请求消息
    Modification : Created function

*****************************************************************************/
XVOID agentUA_DataReqFill(t_XOSCOMMHEAD *pMsgHead, XS32 appHandle, XU32 uiLength, XU8 *pData)
{
	t_DATAREQ tDataReq = {0};

	tSmppLink* pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_DataReqFill XOS_listGetElem failed");
		return;	
	}
	tDataReq.linkHandle = pSmpplink->linkHandle;
	tDataReq.dstAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->peerMasterIp));
	tDataReq.dstAddr.port = pSmpplink->peerPort;

	tDataReq.msgLenth = uiLength;
	tDataReq.pData = (XS8 *)XOS_MemMalloc(FID_UA, uiLength);
    if(NULL == tDataReq.pData)
    {

		XOS_Trace(MD(FID_UA, PL_ERR), "XOS_MemMalloc failed");
		return;	
    }
    agentua_EncodeTcpe(pData, uiLength);
	XOS_MemCpy(tDataReq.pData, pData, uiLength);
	XOS_MemCpy(pMsgHead->message, &tDataReq, sizeof(t_DATAREQ));

    return;
}

XS32 agentUA_LinkStartFill(t_XOSCOMMHEAD *pMsgHead, XS32 appHandle)
{
	t_LINKSTART tLinkStart = {0};
	tSmppLink* pSmpplink;

	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), " XOS_listGetElem failed");
		return XERROR;	
	}
	tLinkStart.linkHandle = pSmpplink->linkHandle;

	switch(pSmpplink->linkType)
	{
		/*UDP方式地填充*/
	case eUDP:
		tLinkStart.linkStart.udpStart.myAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->smcMasterIp));  
		tLinkStart.linkStart.udpStart.myAddr.port = pSmpplink->smcPort;
		tLinkStart.linkStart.udpStart.peerAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->peerMasterIp));				
		tLinkStart.linkStart.udpStart.peerAddr.port = pSmpplink->peerPort;
		XOS_MemCpy(pMsgHead->message, &tLinkStart, sizeof(t_LINKSTART));
		break;

		/*TCP Server方式地填充*/
	case eTCPServer:
		tLinkStart.linkStart.tcpServerStart.allownClients = MAX_SMPP_LINK_COUNT;
		tLinkStart.linkStart.tcpServerStart.myAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->smcMasterIp));
		tLinkStart.linkStart.tcpServerStart.myAddr.port = pSmpplink->smcPort;
		XOS_MemCpy(pMsgHead->message, &tLinkStart, sizeof(t_LINKSTART));
		break;

		/*TCP Client方式地填充，注意客户端端口号为0*/
	case eTCPClient:	
		tLinkStart.linkStart.tcpClientStart.myAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->smcMasterIp));
		tLinkStart.linkStart.tcpClientStart.myAddr.port = 0;
		tLinkStart.linkStart.tcpClientStart.peerAddr.ip = (XU32)ntohl(inet_addr((char*)pSmpplink->peerMasterIp));
		tLinkStart.linkStart.tcpClientStart.peerAddr.port = pSmpplink->peerPort;
		XOS_MemCpy(pMsgHead->message, &tLinkStart, sizeof(t_LINKSTART));
		break;

	default:
		XOS_Trace(MD(FID_UA, PL_ERR), "LinkType %d Err!\n", pSmpplink->linkType);
		return XERROR;		
	}

	return XSUCC;
}
XS32 agentUA_HlrHeartMsgPro(t_DATAIND *pMsg, XS32 iCommandID)
{
    
	XS32 appHandle;
	XS64 appTempHandle;
	XU32 destFid = 0;
	XS32 ret = XERROR;
    tSmppLink* pSmpplink;
    XU8 eLinkType; 


	appTempHandle = (XS64)pMsg->appHandle;
	appHandle = Fuction64To32(appTempHandle);
	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "agentUA_HlrMsgProc XOS_listGetElem failed");
		return XERROR;
	}
    eLinkType = pSmpplink->linkType;
	if (iCommandID == E_AGENTUA_HEART_REQ)
	{
	    agentUA_HeartSendRsp(FID_UA, appHandle);
		XOS_Trace(MD(FID_UA, PL_MIN), "Receiving enquirelink_rsp!");
        pSmpplink->usRepeat = 0;

    }
    else {

	    agentUA_HeartSendReq(FID_UA, appHandle);
		XOS_Trace(MD(FID_UA, PL_MIN), "Receiving enquirelink_rsp!");
        pSmpplink->usRepeat = 0;

    }
    return XSUCC;
}
XS32 agentUA_HlrMsgProc(t_DATAIND *pMsg)
{
	XS32 iCommandID;
	XS32 appHandle;
	XS64 appTempHandle;
	XU32 destFid = 0;
	XS32 ret = XERROR;
    tSmppLink* pSmpplink;


	appTempHandle = (XS64)pMsg->appHandle;
	appHandle = Fuction64To32(appTempHandle);
	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "agentUA_HlrMsgProc XOS_listGetElem failed");
		return XERROR;
	}


    XOS_MemCpy(&iCommandID, pMsg->pData+sizeof(XS32), sizeof(XS32));
	iCommandID = XOS_NtoHl(iCommandID);

    switch(iCommandID)
    {
        case E_AGENTUA_HEART_REQ:
        case E_AGENTUA_HEART_RSP:
            agentUA_HlrHeartMsgPro(pMsg, iCommandID );
            break;
        default:
            break;

    }

    return XSUCC;
}
/*****************************************************************************
 函 数 名  : agentUA_UdpMsgPro
 功能描述  : udp消息处理
 输入参数  : t_DATAIND *pMsg         
             HLINKHANDLE linkHandle  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 agentUA_UdpMsgPro(t_DATAIND *pMsg, HLINKHANDLE linkHandle)
{
    t_Sj3_Oper_Rsp *pHlrMsg = (t_Sj3_Oper_Rsp*)pMsg->pData;
    t_AGENTUA agentMsg = {0};
    t_AGENTUA *pAgentMsg = NULL;
    t_XOSCOMMHEAD* pXosHead;
    XU16 ulLen = sizeof(t_Sj3_Public_Head)+sizeof(t_Sj3_Oper_Head)+XOS_NtoHs(pHlrMsg->OpHead.usLen);

    //XOS_Trace(MD(FID_UA, PL_DBG), "        TMP1 = %u", pHlrMsg->Ph.TMP1);
    XOS_Trace(MD(FID_UA, PL_DBG), "    PackType = %u", pHlrMsg->Ph.ucPackType);
    XOS_Trace(MD(FID_UA, PL_DBG), "       DlgId = %u", XOS_NtoHs(pHlrMsg->Ph.usDlgId));
    XOS_Trace(MD(FID_UA, PL_DBG), "OperateObjId = %u", pHlrMsg->OpHead.ucOperateObjId);
    XOS_Trace(MD(FID_UA, PL_DBG), " OperateType = %u", pHlrMsg->OpHead.ucOperateTypeId);
    XOS_Trace(MD(FID_UA, PL_DBG), "       usLen = %u", XOS_NtoHs(pHlrMsg->OpHead.usLen));
    XOS_Trace(MD(FID_UA, PL_DBG), "   HlrMsgLen = %u", ulLen);
    if(pHlrMsg->OpHead.ucOperateObjId != e_OperObj)
    {

		XOS_Trace(MD(FID_UA,PL_ERR), " ucOperateObjId is Err!");
		return XERROR;
    }
    pXosHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_AGENTUA));
	if (XNULLP == pXosHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
	}

	pXosHead->datasrc.FID =  FID_UA;
	pXosHead->datadest.FID = FID_HLR;
	pXosHead->datadest.PID = XOS_GetLocalPID();
	pXosHead->datasrc.PID = XOS_GetLocalPID();
	
	pXosHead->prio = eNormalMsgPrio;

    
    //agentMsg.peerType = PEER_HLR;
    agentMsg.msgLenth = ulLen;
    /*该内存块在对端释放*/
    agentMsg.pData = (XCHAR *)XOS_MemMalloc(FID_UA, ulLen);
    if(NULL == agentMsg.pData)
    {
		XOS_Trace(MD(FID_UA,PL_ERR), " XOS_MemMalloc is Err!");
        XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;

    }
    XOS_MemCpy(agentMsg.pData, pMsg->pData, ulLen);
    XOS_MemCpy(pXosHead->message, &agentMsg, sizeof(t_AGENTUA));


    if(XSUCC != XOS_MsgSend(pXosHead))
	{
		XOS_PRINT(MD(FID_UA, PL_ERR),"agentUA_UdpMsgPro send smppua msg to UA failed.");
		/*发送失败需要释放内存*/
		XOS_MemFree(FID_UA, agentMsg.pData);
		XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;
	}
    
    XOS_Trace(MD(FID_UA, PL_DBG), " Send msg to UA successfully!");
	return XSUCC;

}

XS32 agentUA_SingleMsgPro(t_DATAIND *pMsg, XU32 HlrMsgLen)
{
    t_Sj3_Oper_Rsp *pHlrMsg = (t_Sj3_Oper_Rsp*)pMsg->pData;
    t_AGENTUA agentMsg = {0};
    t_AGENTUA *pAgentMsg = NULL;
    t_XOSCOMMHEAD* pXosHead;
    XU16 ulLen = LENGTH_HEAD+XOS_NtoHs(pHlrMsg->OpHead.usLen)+LENGTH_TAIL;

    //XOS_Trace(MD(FID_UA, PL_DBG), "      TMP1 = %u", pHlrMsg->Ph.TMP1);
    XOS_Trace(MD(FID_UA, PL_DBG), "      PackType = %u", pHlrMsg->Ph.ucPackType);
    XOS_Trace(MD(FID_UA, PL_DBG), "         DlgId = %u", XOS_NtoHs(pHlrMsg->Ph.usDlgId));
    XOS_Trace(MD(FID_UA, PL_DBG), "  OperateObjId = %u", pHlrMsg->OpHead.ucOperateObjId);
    XOS_Trace(MD(FID_UA, PL_DBG), "   OperateType = %u", pHlrMsg->OpHead.ucOperateTypeId);
    XOS_Trace(MD(FID_UA, PL_DBG), "       ucIsEnd = %u", pHlrMsg->OpHead.ucIsEnd);
    XOS_Trace(MD(FID_UA, PL_DBG), "         usLen = %u", XOS_NtoHs(pHlrMsg->OpHead.usLen));
    XOS_Trace(MD(FID_UA, PL_DBG), "         HlrMsgLen = %u", ulLen);
    XOS_Trace(MD(FID_UA, PL_DBG), "         HlrMsgLen = %u", HlrMsgLen);
    
    if(pHlrMsg->OpHead.ucOperateObjId != e_OperObj)
    {

		XOS_Trace(MD(FID_UA,PL_ERR), " ucOperateObjId is Err!");
		return XERROR;
    }
    pXosHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_AGENTUA));
	if (XNULLP == pXosHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
	}

	pXosHead->datasrc.FID =  FID_UA;
	pXosHead->datadest.FID = FID_HLR;
	pXosHead->datadest.PID = XOS_GetLocalPID();
	pXosHead->datasrc.PID = XOS_GetLocalPID();
	
	pXosHead->prio = eNormalMsgPrio;

    
    //agentMsg.peerType = PEER_HLR;
    agentMsg.msgLenth = ulLen;
    /*该内存块在对端释放*/
    agentMsg.pData = (XCHAR *)XOS_MemMalloc(FID_UA, ulLen);
    if(NULL == agentMsg.pData)
    {
		XOS_Trace(MD(FID_UA,PL_ERR), " XOS_MemMalloc is Err!");
        XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;

    }
    XOS_MemCpy(agentMsg.pData, pMsg->pData, ulLen);
    XOS_MemCpy(pXosHead->message, &agentMsg, sizeof(t_AGENTUA));


    if(XSUCC != XOS_MsgSend(pXosHead))
	{
		XOS_PRINT(MD(FID_UA, PL_ERR),"Send msg to HLR failed.");
		/*发送失败需要释放内存*/
		XOS_MemFree(FID_UA, agentMsg.pData);
		XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;
	}
    
    XOS_Trace(MD(FID_UA, PL_DBG), " Send msg to HLR successfully!");
	return XSUCC;
}

XS32 agentUA_UdpMsgPro_test(t_DATAIND *pMsg, HLINKHANDLE linkHandle)
{
    t_Sj3_Oper_Rsp *pHlrMsg = (t_Sj3_Oper_Rsp*)pMsg->pData;
    t_AGENTUA agentMsg = {0};
    t_AGENTUA *pAgentMsg = NULL;
    t_XOSCOMMHEAD* pXosHead;
    XU16 ulLen = sizeof(t_Sj3_Public_Head)+sizeof(t_Sj3_Oper_Head)+XOS_NtoHs(pHlrMsg->OpHead.usLen);

    XOS_Trace(MD(FID_UA, PL_ERR), "      PackType = %u", pHlrMsg->Ph.ucPackType);
    XOS_Trace(MD(FID_UA, PL_ERR), "         DlgId = %u", XOS_NtoHs(pHlrMsg->Ph.usDlgId));
    XOS_Trace(MD(FID_UA, PL_ERR), "  OperateObjId = %u", pHlrMsg->OpHead.ucOperateObjId);
    XOS_Trace(MD(FID_UA, PL_ERR), "   OperateType = %u", pHlrMsg->OpHead.ucOperateTypeId);
    XOS_Trace(MD(FID_UA, PL_ERR), "         usLen = %u", XOS_NtoHs(pHlrMsg->OpHead.usLen));

    pHlrMsg->Ph.ucPackType = SJ_RSP;
    pHlrMsg->OpHead.ucIsEnd = 0x01;
    pXosHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_AGENTUA));
	if (XNULLP == pXosHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
	}

	pXosHead->datasrc.FID = FID_MQTT;
	pXosHead->datadest.FID = FID_UA;
	pXosHead->datadest.PID = XOS_GetLocalPID();
	pXosHead->datasrc.PID = XOS_GetLocalPID();
	
	//pXosHead->msgID = eSendData;
	pXosHead->prio = eNormalMsgPrio;

    
    agentMsg.peerType = PEER_HLR;
    agentMsg.msgLenth = XOS_HtoNs(ulLen);
    /*该内存块在对端释放*/
    agentMsg.pData = (XCHAR *)XOS_MemMalloc(FID_UA, ulLen);
    XOS_MemCpy(agentMsg.pData, pMsg->pData, ulLen);
    XOS_MemCpy(pXosHead->message, &agentMsg, sizeof(t_AGENTUA));


    if(XSUCC != XOS_MsgSend(pXosHead))
	{
		XOS_PRINT(MD(FID_UA, PL_ERR),"agentUA_UdpMsgPro send smppua msg to UA failed.");
		
		XOS_MemFree(FID_UA, agentMsg.pData);
		XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;
	}
    
    XOS_Trace(MD(FID_UA, PL_ERR), " Send msg to UA successfully!");
	return XSUCC;
}
/*****************************************************************************
 函 数 名  : agentUA_TcpMsgPro
 功能描述  : tcp消息分片处理
 输入参数  : t_DATAIND *pMsg         
             HLINKHANDLE linkHandle  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 agentUA_TcpMsgPro(t_DATAIND *pMsg, HLINKHANDLE linkHandle)
{
	XU32 uiDataLength = 0;    /*缓冲区中剩余数据地长度*/
	XU32 uiBufferLength = 0;  /*SMPP_tMsgBuffer中数据地长度*/
	XS32 ret = XERROR;
	XS32 iMsgBufferIndex;
	MSG_BUFFER_T tMsgBuffer = {0};
	MSG_BUFFER_T *ptepMsgBuffer;
	t_DATAIND tSingleMsg;
    XU32 HlrMsgHeadLen = LENGTH_HEAD;
    XU32 HlrMsgTailLen = LENGTH_TAIL;
    XU32 HlrMsgLen = 0;
    t_Sj3_Oper_Rsp *pHlrMsg = NULL;

	/*匹配和链路对应的缓冲区*/
	agentUA_MsgBufferMatch(linkHandle, &iMsgBufferIndex);
	if (-1 == iMsgBufferIndex)
	{
		/*不存在该链路缓冲区的*/
		tMsgBuffer.linkHandle = linkHandle;
		tMsgBuffer.uiLength = 0;
		iMsgBufferIndex = XOS_listAddTail(smppUA_htMsgBufferList, &tMsgBuffer);
		ptepMsgBuffer = (MSG_BUFFER_T *)XOS_listGetElem(smppUA_htMsgBufferList, iMsgBufferIndex);

	}
	else
	{
		ptepMsgBuffer = (MSG_BUFFER_T *)XOS_listGetElem(smppUA_htMsgBufferList, iMsgBufferIndex);
	}	

	/*先赋长度值，进行拷贝*/
	uiBufferLength = ptepMsgBuffer->uiLength;	

	/*将数据拷贝到消息缓存*/
	if(uiBufferLength + pMsg->dataLenth > MSGBUFFER_MAX_DEF)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlMsgProc]There is Invalid Msg, Clear the Buffer!");
		XOS_MemSet(ptepMsgBuffer->cTemBuffer, 0, MSGBUFFER_MAX_DEF);
		ptepMsgBuffer->uiLength = 0;
		return XERROR;
	}
	XOS_MemCpy(ptepMsgBuffer->cTemBuffer+uiBufferLength, pMsg->pData, pMsg->dataLenth);
	ptepMsgBuffer->uiLength = uiBufferLength + pMsg->dataLenth;

	/*拷贝后进行赋正确的长度值*/
	uiBufferLength = ptepMsgBuffer->uiLength;
	uiDataLength = uiBufferLength;

    if(uiDataLength < HlrMsgHeadLen)
	{	
		return XSUCC;
	}

	/*取完整消息长度*/
    pHlrMsg = (t_Sj3_Oper_Rsp *)ptepMsgBuffer->cTemBuffer;
    HlrMsgLen = HlrMsgHeadLen + XOS_NtoHs(pHlrMsg->OpHead.usLen) + HlrMsgTailLen;
	if (HlrMsgLen > MSGBUFFER_MAX_DEF)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlMsgProc]There is Invalid Msg, Clear the Buffer!");
		XOS_MemSet(ptepMsgBuffer->cTemBuffer, 0, MSGBUFFER_MAX_DEF);
		ptepMsgBuffer->uiLength = 0;
		return XERROR;
	}

	/*判断缓冲区是否有一个消息的长度*/
	if (uiDataLength < HlrMsgLen)
	{
		return XSUCC;
	}
    XOS_Trace(MD(FID_UA, PL_DBG), "uiDataLength = %u, HlrMsgLen = %u.", uiDataLength, HlrMsgLen);
	while (uiDataLength >= HlrMsgLen)
	{
	    
	    /*处理粘包消息*/
		tSingleMsg.appHandle = pMsg->appHandle;
		tSingleMsg.peerAddr.ip = pMsg->peerAddr.ip;
		tSingleMsg.peerAddr.port = pMsg->peerAddr.port;
		tSingleMsg.dataLenth = HlrMsgLen;
		tSingleMsg.pData = (XS8 *)XOS_MemMalloc(FID_UA, HlrMsgLen);
		XOS_MemCpy(tSingleMsg.pData, ptepMsgBuffer->cTemBuffer , HlrMsgLen);

		ret = agentUA_SingleMsgPro(&tSingleMsg, HlrMsgLen);  

        
		XOS_MemFree(FID_UA, tSingleMsg.pData);
        tSingleMsg.pData = NULL;

		uiDataLength = uiDataLength - HlrMsgLen;
		ptepMsgBuffer->uiLength = uiDataLength;

		/*缓冲区移动*/
		if (uiDataLength > 0)
		{
			XOS_MemMove(ptepMsgBuffer->cTemBuffer , ptepMsgBuffer->cTemBuffer + HlrMsgLen, uiDataLength);
			XOS_MemSet(ptepMsgBuffer->cTemBuffer + uiDataLength, 0, MSGBUFFER_MAX_DEF - uiDataLength);
		} 
        else 
		{
			XOS_MemSet(ptepMsgBuffer->cTemBuffer, 0, MSGBUFFER_MAX_DEF);
			return XSUCC;
		}

		/*判断缓冲区是否存有消息长度字段的值，没有直接返回*/
		if (uiDataLength < HlrMsgHeadLen)
		{	
			break;
		}
		/*取完整消息长度*/
        pHlrMsg = (t_Sj3_Oper_Rsp *)ptepMsgBuffer->cTemBuffer;
        HlrMsgLen = HlrMsgHeadLen + XOS_NtoHs(pHlrMsg->OpHead.usLen) + HlrMsgTailLen;
		if ( HlrMsgLen > MSGBUFFER_MAX_DEF)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlMsgProc]There is Invalid Msg, Clear the Buffer!");
			XOS_MemSet(ptepMsgBuffer->cTemBuffer, 0, MSGBUFFER_MAX_DEF);
			ptepMsgBuffer->uiLength = 0;
			return XERROR;
		}

		/*判断缓冲区是否有一个消息的长度*/
		if (uiDataLength < HlrMsgLen)
		{
			break;
		}
	}  

	return XSUCC;
    
}
/*****************************************************************************
 函 数 名  : agentUA_NtlMsgProc
 功能描述  : 消息接收函数
 输入参数  : t_DATAIND *pMsg  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 agentUA_NtlMsgProc(t_DATAIND *pMsg)
{
	XS32 ret = XERROR;
	XS32 appHandle = 0;
	XS64 appTempHandle = 0;
	HLINKHANDLE linkHandle;
	tSmppLink* pSmpplink = XNULL;

	appTempHandle = (XS64)pMsg->appHandle;
	appHandle = Fuction64To32(appTempHandle);
	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlMsgProc XOS_listGetElem failed");
		return XERROR;
	}
	linkHandle = pSmpplink->linkHandle;

    if(eUDP == pSmpplink->linkType)
    {
        return agentUA_UdpMsgPro(pMsg, linkHandle);

    }
    else {

        return agentUA_TcpMsgPro(pMsg, linkHandle);

    }


	return ret;

}
/*****************************************************************************
 函 数 名  : agentUA_NtlRspProc
 功能描述  : 和ntl模块交互函数处理
 输入参数  : t_XOSCOMMHEAD *pMsgHead  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 agentUA_NtlRspProc(t_XOSCOMMHEAD *pMsgHead)
{

	t_LINKRELEASE tLinkRelease = {0};
	t_LINKINITACK *ptLinkInitAck    = XNULL;        /*初始化确认信息*/
	t_STARTACK *ptLinkStartAck      = XNULL;          /*LinkStart确认信息*/
	t_CONNIND *ptConnInd            = XNULL;                /*连接确认信息*/
	t_SENDERROR *ptSendError        = XNULL;            /*发送数据成功失败地确认信息*/
	t_DATAIND *ptDataInd            = XNULL;                /*有消息上来地数据结构定义*/
	t_LINKCLOSEIND *ptLinkCloseInd  = XNULL;      /*关闭链路地确认信息*/
	t_XOSCOMMHEAD *ptCommHead       = XNULL;
	tSmppLink* pSmpplink            = XNULL;
    t_BACKPARA tBackPara         = {0};
    t_BACKPARA tBackParaRsp      = {0};
	XS8 tempAddr[MAX_HOST_DEF]   = {0};

	XS32 length = 0;
	XS32 itemp = 0;
	XS32 ret = XERROR; 
	XS32 appHandle = 0;
	XS64 appTempHandle = 0;


	switch(pMsgHead->msgID)
	{
		/*对消息eInitAck地处理*/
	case eInitAck:
		ptLinkInitAck = (t_LINKINITACK *)(pMsgHead->message);
		if (eFAIL == ptLinkInitAck->lnitAckResult)
		{
			XOS_Trace(MD(FID_UA,PL_ERR),"InitAck is Err!");
			return XERROR;
		}

		appTempHandle = (XS64)ptLinkInitAck->appHandle;
		appHandle = Fuction64To32(appTempHandle);
		pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
		if (pSmpplink == XNULL)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlRspProc XOS_listGetElem failed");
			return XERROR;
		}
		pSmpplink->linkHandle = ptLinkInitAck->linkHandle;

		/*收到linkinit后，发送linkstartreq，请求连接*/
		ptCommHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_LINKSTART));
		if (XNULLP == ptCommHead)
		{
			XOS_Trace(MD(FID_UA,PL_ERR), "[smppUA_NtlRspProc] MsgMemMalloc is Err!");
			return XERROR;
		}

		ptCommHead->datasrc.FID = FID_UA;
		ptCommHead->datadest.FID = FID_NTL;
		ptCommHead->msgID = eLinkStart;
		ptCommHead->prio = eNormalMsgPrio;  
		agentUA_LinkStartFill(ptCommHead, appHandle);

		ret = XOS_MsgSend(ptCommHead);   
		if (XERROR  == ret)
		{
			/*释放内存*/
			XOS_MemFree(FID_UA, ptCommHead);           
			XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlRspProc] MsgSend is Err!");
			return XERROR;
		}
		break;

	case eStartAck:
		ptLinkStartAck = (t_STARTACK *)pMsgHead->message;      
		if (eFAIL == ptLinkStartAck->linkStartResult)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlRspProc] StartAck is Err!");
			return XERROR;
		}

		appTempHandle = (XS64)ptLinkStartAck->appHandle;
		appHandle = Fuction64To32(appTempHandle);
		pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
		if (pSmpplink == XNULL)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlRspProc XOS_listGetElem failed");
			return XERROR;
		}
        pSmpplink->linkStatus = (eSUCC == ptLinkStartAck->linkStartResult) ? LINKSTATUS_OK : LINKSTATUS_ERROR;
        pSmpplink->usRepeat = 0;
		if(eSUCC == ptLinkStartAck->linkStartResult && pSmpplink->linkType == eTCPClient)
		{   
		    XOS_Trace(MD(FID_UA, PL_DBG), "agentua link is ok!!");
            pSmpplink->smcPort = ptLinkStartAck->localAddr.port;


            
    		/*启动 发送链路心跳定时器*/
    		tBackPara.para1 = TIMER_HEART_REQ_SEND;
    		tBackPara.para2 = appHandle;

    		ret = XOS_TimerCreate(FID_UA, &(pSmpplink->hTimerHandle), TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackPara);
    		if (XERROR == ret)
    		{
    			XOS_Trace(MD(FID_UA,PL_ERR), "TcpClient eHeartBeat TimerCreate is Err!");
    			return XERROR;
    		}

    		ret = XOS_TimerBegin(FID_UA, pSmpplink->hTimerHandle, ENQUIRELINK_INTERVAL_TIME_OUT);//EnquireLink间隔
    		if (XERROR == ret)
    		{
    			XOS_Trace(MD(FID_UA,PL_ERR), "TcpClient eHeartBeat TimerBegin is Err!");
    			return XERROR;
    		}

            /*启动 检查链路心跳定时器*/
            tBackParaRsp.para1 = TIMER_HEART_WATCHER;
			tBackParaRsp.para2 = appHandle;

    		ret = XOS_TimerCreate(FID_UA, &(pSmpplink->TimerCheck), TIMER_TYPE_LOOP, TIMER_PRE_LOW, &tBackParaRsp);
    		if (XERROR == ret)
    		{
    			XOS_Trace(MD(FID_UA,PL_ERR), "TcpClient eHeartBeat TimerCreate is Err!");
    			return XERROR;
    		}

    		ret = XOS_TimerBegin(FID_UA, pSmpplink->TimerCheck, ENQUIRELINK_INTERVAL_TIME_OUT);//EnquireLink间隔
    		if (XERROR == ret)
    		{
    			XOS_Trace(MD(FID_UA,PL_ERR), "TcpClient eHeartBeat TimerBegin is Err!");
    			return XERROR;
    		}
            
		}
		break;

	case eConnInd:  
		ptConnInd = (t_CONNIND *)(pMsgHead->message);
		appTempHandle = (XS64)ptConnInd->appHandle;
		appHandle = Fuction64To32(appTempHandle);

		pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
		if (pSmpplink == XNULL)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlRspProc XOS_listGetElem failed");
			return XERROR;
		}
		if (ptConnInd->peerAddr.ip != ntohl(inet_addr((char*)pSmpplink->peerMasterIp)))
		{
			XOS_Trace(MD(FID_UA,PL_ERR), "[smppUA_NtlRspProc] eConnInd peer Ip is Err and eConnInd ip is %u, pSmpplink->peerMasterIp is %s!", ptConnInd->peerAddr.ip, pSmpplink->peerMasterIp);
			return XERROR;
		}
		pSmpplink->peerPort = ptConnInd->peerAddr.port;
		break;

	case eDataInd:
		ptDataInd = (t_DATAIND *)(pMsgHead->message);  
		appTempHandle = (XS64)ptDataInd->appHandle;
		appHandle = Fuction64To32(appTempHandle);

		pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
		if (pSmpplink == XNULL)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlRspProc XOS_listGetElem failed");
			return XERROR;
		}
		if (ptDataInd->peerAddr.ip != ntohl(inet_addr((char*)pSmpplink->peerMasterIp)) || ptDataInd->peerAddr.port != pSmpplink->peerPort)
		{
			XOS_Trace(MD(FID_UA,PL_ERR), "[smppUA_NtlRspProc] eDataInd peer Ip or Port is Err!");
			return XERROR;
		}
		ret = agentUA_NtlMsgProc(ptDataInd);           
		XOS_MemFree(FID_UA, ptDataInd->pData);           	
		break;

	case eErrorSend:
		ptSendError=(t_SENDERROR *)(pMsgHead->message);
		appTempHandle = (XS64)ptSendError->userHandle;  
		appHandle = Fuction64To32(appTempHandle);
		XOS_Trace(MD(FID_UA, PL_ERR),"NTL Send eErrorSend appHandle[%u]!", appHandle);			
		break;

	case eStopInd:
		ptLinkCloseInd=(t_LINKCLOSEIND *)pMsgHead->message;
		appTempHandle = (XS64)ptLinkCloseInd->appHandle;
		appHandle = Fuction64To32(appTempHandle);
		pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
		if (pSmpplink == XNULL)
		{
			XOS_Trace(MD(FID_UA, PL_ERR), "smppUA_NtlRspProc XOS_listGetElem failed");
			return XERROR;
		}
        
     #if 0   
		if (ptLinkCloseInd->peerAddr.ip != ntohl(inet_addr((char*)pSmpplink->peerMasterIp)) || ptLinkCloseInd->peerAddr.port != pSmpplink->peerPort)
		{
			XOS_Trace(MD(FID_UA,PL_ERR), "[smppUA_NtlRspProc] eStopInd peer Ip or Port is Err!%s", pSmpplink->peerMasterIp);
            XOS_Trace(MD(FID_UA,PL_ERR), "peerIp = %u,LocalIp=%u  peerPort=%u,LocalPort=%u.", ptLinkCloseInd->peerAddr.ip,
                                            ntohl(inet_addr((char*)pSmpplink->peerMasterIp)),
                                            ptLinkCloseInd->peerAddr.port, pSmpplink->peerPort);
			return XERROR;
		}
      #endif
        pSmpplink->linkStatus = LINKSTATUS_ERROR;
		if (eTCPClient == pSmpplink->linkType)
		{ 
			/*释放HeartBeat定时器*/
			XOS_TimerEnd(FID_UA, pSmpplink->hTimerHandle);
			XOS_TimerDelete(FID_UA, pSmpplink->hTimerHandle);
			pSmpplink->hTimerHandle = NULL;

			/*释放Response定时器*/
			XOS_TimerEnd(FID_UA, pSmpplink->TimerCheck);
			XOS_TimerDelete(FID_UA, pSmpplink->TimerCheck);
			pSmpplink->TimerCheck = NULL;

			/*向NTL发送LinkRelease 请求消息*/
			ptCommHead=(t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA,sizeof(t_LINKRELEASE));
			if (XNULLP == ptCommHead)
			{
				XOS_Trace(MD(FID_UA, PL_WARN), "[smppUA_NtlRspProc] XOS_MsgMemMalloc is Err!");
				return XERROR;
			}

			ptCommHead->datasrc.FID = FID_UA;
			ptCommHead->datadest.FID = FID_NTL;
			ptCommHead->msgID = eLinkRelease;
			ptCommHead->prio = eNormalMsgPrio;
			tLinkRelease.linkHandle=pSmpplink->linkHandle;
			XOS_MemCpy(pMsgHead->message, &tLinkRelease, sizeof(t_LINKRELEASE));

			ret=XOS_MsgSend(ptCommHead);
			if (XERROR == ret)
			{
				/*释放内存*/
				XOS_MemFree(FID_UA, ptCommHead);
				XOS_Trace(MD(FID_UA, PL_WARN), "[smppUA_NtlRspProc] XOS_MsgSend is Err!");
				return XERROR;
			}

			/*重新发起连接服务器*/
			ptCommHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_LINKSTART));
			if (XNULLP == ptCommHead)
			{
				XOS_Trace(MD(FID_UA,PL_ERR), "[smppUA_NtlRspProc] MsgMemMalloc is Err!");
				return XERROR;
			}

			ptCommHead->datasrc.FID = FID_UA;
			ptCommHead->datadest.FID = FID_NTL;
			ptCommHead->msgID = eLinkInit;
			ptCommHead->prio = eNormalMsgPrio;  
			agentUA_LinkStartFill(ptCommHead, appHandle);

			ret = XOS_MsgSend(ptCommHead);   
			if (XERROR  == ret)
			{
				/*释放内存*/
				XOS_MemFree(FID_UA, ptCommHead);           
				XOS_Trace(MD(FID_UA, PL_ERR), "[smppUA_NtlRspProc] MsgSend is Err!");
				return XERROR;
			}
		}
		break;

	default:
		XOS_Trace(MD(FID_UA,PL_ERR),"[smppUA_NtlRspProc] Received NTLType is Err!");
		return XERROR;
	}

	return XSUCC;
}
