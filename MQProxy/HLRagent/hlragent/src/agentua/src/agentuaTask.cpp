/******************************************************************************

                  版权所有 (C), xinwei, xinwei

 ******************************************************************************
  文 件 名   : agentuaTask.cpp
  版 本 号   : 初稿
  作    者   : wangdanfeng
  生成日期   : 2015年7月21日
  最近修改   :
  功能描述   : ua链路主任务启动文件
  函数列表   :
              agentHLR_Encode
              agentUA_InitProc
              agentUA_Linkadd
              agentUA_Linkdelete
              agentUA_NoticeProc
              agentUA_ShowLinkTable
              agentUA_Test
              agentUA_TimeoutProc
              agentUA_TimeOutSendHeartReqPro
              agentUA_XosMsgProc
              UdpMsgPro_test
  修改历史   :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "agentuaTask.h"
#include "agentuaOam.h"
#include "agentProtocol.h"
#include "agentuaXos.h"
#include "fid_def.h"
#include "xosshell.h"
#include "agentTrace.h"
#include "smu_sj3_type.h"
/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/
LinkInfo  g_hlrLinkInfo = {0};

XS32 agentHLR_Encode(XU8 *buf, XU8 tag, XU8 *Value, XU32 valueLen)
{
	XU16 len = 0;
	// tag 
	*(XU8*)(buf + len) = tag;
	len += 1;
	// len
	*(XU16*)(buf + len) = XOS_HtoNs(valueLen) ;
	len += 2;
	// v
	XOS_MemCpy((XU8 *)(buf + len),Value,valueLen);
	len += valueLen;
	return len;
    

}
/*****************************************************************************
 函 数 名  : UdpMsgPro_test
 功能描述  : 测试函数
 输入参数  : XU8 ucPackType  
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XS32 UdpMsgPro_test(XU8 ucPackType)
{
    t_Sj3_Oper_Rsp *pHlrMsg;
    t_AGENTUA agentMsg = {0};
    t_XOSCOMMHEAD* pXosHead;
    XU32 UID = 10;
    XCHAR  UIDstr[4] = {0};
    TXdbHashStat tStaticInfo = {0};
    XU16 len = 0;

    XOS_MemCpy(tStaticInfo.tel, "13120199852", XOS_StrLen("13120199852"));


    
    XU16 ulLen = sizeof(t_Sj3_Public_Head)+sizeof(t_Sj3_Oper_Head)+(3+ 4 + 3 + sizeof(TXdbHashStat));
    agentMsg.msgLenth = ulLen;
    pHlrMsg = (t_Sj3_Oper_Rsp *)XOS_MemMalloc(FID_UA, ulLen);

    agentMsg.pData = (XCHAR *)pHlrMsg;
    pHlrMsg->Ph.ucPackType = ucPackType;
    pHlrMsg->Ph.usDlgId= XOS_HtoNs(1);
    
    pHlrMsg->OpHead.ucOperateObjId = e_OperObj;
    pHlrMsg->OpHead.ucOperateTypeId = e_Qry_OperType;
    pHlrMsg->OpHead.ucIsEnd = 0x01;
    pHlrMsg->OpHead.usLen = XOS_HtoNs(3+ 4 + 3 + sizeof(TXdbHashStat));

    UID = XOS_HtoNl(UID);
    
    //*(XU32*)UIDstr = UID;

    XOS_MemCpy(UIDstr, &UID, sizeof(XU32));
    
    len += agentHLR_Encode(pHlrMsg->usBuf, e_UID_TAG, (XU8*)UIDstr, sizeof(XU32));
    len += agentHLR_Encode(pHlrMsg->usBuf+len, e_STATIC_TAG, (XU8*)&tStaticInfo, sizeof(TXdbHashStat));


    XOS_Trace(MD(FID_UA,PL_DBG), " UIDstr = %u, len = %u, OpHead.usLen = %u", 
                                    *(XU32 *)UIDstr, len, (3+ 4 + 3 + sizeof(TXdbHashStat)));
    pXosHead = (t_XOSCOMMHEAD *)XOS_MsgMemMalloc(FID_UA, sizeof(t_AGENTUA));
	if (XNULLP == pXosHead)
	{
		XOS_Trace(MD(FID_UA,PL_ERR), " MsgMemMalloc is Err!");
		return XERROR;
	}

	pXosHead->datasrc.FID = FID_HLR;
	pXosHead->datadest.FID = FID_UA;
	pXosHead->datadest.PID = XOS_GetLocalPID();
	pXosHead->datasrc.PID = XOS_GetLocalPID();
	
   	pXosHead->prio = eNormalMsgPrio;



    XOS_MemCpy(pXosHead->message, &agentMsg, sizeof(t_AGENTUA));


    if(XSUCC != XOS_MsgSend(pXosHead))
	{
		XOS_PRINT(MD(FID_UA, PL_ERR),"agentUA_UdpMsgPro send smppua msg to UA failed.");
		
		XOS_MemFree(FID_UA, agentMsg.pData);
		XOS_MsgMemFree(FID_UA, pXosHead);
		return XERROR;
	}
    
    XOS_Trace(MD(FID_UA, PL_ERR), " Send msg to FID_UA successfully!");
	return XSUCC;
}
/*****************************************************************************
 函 数 名  : agentUA_Test
 功能描述  : 测试命令
 输入参数  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR **ppArgv    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XVOID agentUA_Test(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
    if(siArgc < 2)
    {
		XOS_CliExtPrintf(pCliEnv, "input command param error!");
		return;

    }
    UdpMsgPro_test(atoi(ppArgv[1]));

}
/*****************************************************************************
 函 数 名  : agentUA_ShowLinkTable
 功能描述  : 链路信息回显
 输入参数  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR **ppArgv    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XVOID agentUA_ShowLinkTable(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
	XS32 i;
	XU32 iListIndex;
	tSmppLink* pSmppLink;

	if (1 != siArgc)
	{
		XOS_CliExtPrintf(pCliEnv, "input command param error, please refer to help command\r\n");
		return;
	}

	if (0 == XOS_listCurSize(gSmppLinkList))
	{
		XOS_CliExtPrintf(pCliEnv, "INFO: there is no link information\r\n");
		return;
	}


	
	iListIndex = XOS_listHead(gSmppLinkList);
	for(i=0;i<XOS_listCurSize(gSmppLinkList);i++)
	{
        pSmppLink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, iListIndex);

        XOS_CliExtPrintf(pCliEnv, "%12s = %u\r\n", "index",pSmppLink->index);
        XOS_CliExtPrintf(pCliEnv, "%12s = %u(%s)\r\n", "peerType",pSmppLink->peerType, 
                                      pSmppLink->peerType == PEER_HLR ? "HLR" : "UDC");

        XOS_CliExtPrintf(pCliEnv, "%12s = %u(%s)\r\n", "linkType",pSmppLink->linkType, 
            pSmppLink->linkType  == eUDP ? "UDP" : (pSmppLink->linkType == eTCPClient? "TCPclient" : "TCPserver"));

        XOS_CliExtPrintf(pCliEnv, "%12s = %s\r\n", "LocalIp",pSmppLink->smcMasterIp);
        XOS_CliExtPrintf(pCliEnv, "%12s = %u\r\n", "LocalPort",pSmppLink->smcPort);
        XOS_CliExtPrintf(pCliEnv, "%12s = %s\r\n", "peerIp",pSmppLink->peerMasterIp);
        XOS_CliExtPrintf(pCliEnv, "%12s = %u\r\n", "peerPort",pSmppLink->peerPort);

        XOS_CliExtPrintf(pCliEnv, "%12s = %u\r\n", "appHandle",iListIndex);
        XOS_CliExtPrintf(pCliEnv, "%12s = %u(%s)\r\n", "linkStatus",pSmppLink->linkStatus,  
                                   pSmppLink->linkStatus == LINKSTATUS_OK ? "OK" : "ERR");
        XOS_CliExtPrintf(pCliEnv, "%12s = %u\r\n", "usRepeat",pSmppLink->usRepeat);
        XOS_CliExtPrintf(pCliEnv, "%12s = %s\r\n", "linkDesc",pSmppLink->linkDesc);
        XOS_CliExtPrintf(pCliEnv, "******************\r\n");


        iListIndex = XOS_listNext(gSmppLinkList, iListIndex);

    }

	XOS_CliExtPrintf(pCliEnv,"(Totle Num = %d)\r\n", XOS_listCurSize(gSmppLinkList));


}

/*****************************************************************************
 函 数 名  : agentUA_Linkadd
 功能描述  : 链路增加
 输入参数  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR **ppArgv    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XVOID agentUA_Linkadd(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{


	t_XOSCOMMHEAD *send_msg =XNULL;
	XS32 size=0;
	AGT_OAM_CFG_REQ_T OamMsg ={0};
	tPeSmppCfgOamTable pcfgoamtable = {0};

	if(8 > siArgc)
	{
		XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!\r\n");
		return;
	}



	OamMsg.uiIndex = 1;
	OamMsg.uiMsgLen = 2;
	OamMsg.uiOperType = OAM_CFG_ADD;
	OamMsg.uiRecNum = 4;
	OamMsg.uiSessionId = 5;
	OamMsg.uiTableId = MML_UA_LINK_TABLE_ID;
	OamMsg.usModuleId = 7;
	OamMsg.usNeId = 8;
	OamMsg.usPid = 9;

	OamMsg.pData = (XS8 *)XOS_MemMalloc(FID_UA, sizeof(tPeSmppCfgOamTable));
	pcfgoamtable.index = atoi(ppArgv[1]);
    pcfgoamtable.linkType = atoi(ppArgv[2]);
	XOS_StrCpy(pcfgoamtable.smcIp, ppArgv[3]);
	pcfgoamtable.smcPort = atoi(ppArgv[4]);

	XOS_StrCpy(pcfgoamtable.peerIp , ppArgv[5]);
	pcfgoamtable.peerPort = atoi(ppArgv[6]);

	XOS_StrCpy(pcfgoamtable.linkDesc, ppArgv[7]);
	pcfgoamtable.linkStatus = LINKSTATUS_ERROR;

	memcpy(OamMsg.pData, &pcfgoamtable, sizeof(tPeSmppCfgOamTable));
	send_msg = XOS_MsgMemMalloc(FID_UA, sizeof(AGT_OAM_CFG_REQ_T));
	if(XNULL == send_msg)
	{
		XOS_PRINT(MD(FID_UA, PL_ERR), "get thread receive msg:msg malloc failed.");
		return;
	}

	XOS_MemCpy(send_msg->message,&OamMsg,sizeof(AGT_OAM_CFG_REQ_T));

	send_msg->datasrc.PID = XOS_GetLocalPID();
	send_msg->datasrc.FID =  FID_UA;
	send_msg->datadest.PID= XOS_GetLocalPID();
	send_msg->datadest.FID = FID_AGENTOAM;
	send_msg->prio = eNormalMsgPrio;
	if ( XSUCC !=  XOS_MsgSend(send_msg))
	{
		XOS_MsgMemFree( FID_UA, send_msg );
		XOS_PRINT(MD(FID_UA, PL_WARN), "XOS_MsgSend,send message  failed.");
	}


    return;
}

XS32 agentUA_TimeOutSendHeartReqPro()
{




	return XSUCC;
}
/*****************************************************************************
 函 数 名  : agentUA_Linkdelete
 功能描述  : 删除链路
 输入参数  : CLI_ENV* pCliEnv  
             XS32 siArgc       
             XCHAR **ppArgv    
 输出参数  : 无
 返 回 值  : 
 调用函数  : 
 被调函数  : 
 
 修改历史      :
  1.日    期   : 2015年7月21日
    作    者   : wangdanfeng
    修改内容   : 新生成函数

*****************************************************************************/
XVOID agentUA_Linkdelete(CLI_ENV* pCliEnv,  XS32 siArgc,  XCHAR **ppArgv)
{
	t_XOSCOMMHEAD *send_msg =XNULL;
	XS32 size=0;
	AGT_OAM_CFG_REQ_T OamMsg ={0};
	tPeSmppCfgOamTable pcfgoamtable = {0};

	if(2 != siArgc)
	{
		XOS_CliExtPrintf(pCliEnv,"input parameter is wrong!\r\n");
		return;
	}

	memset(&OamMsg, 0 ,sizeof(OamMsg));

	OamMsg.uiIndex = 1;
	OamMsg.uiMsgLen = 2;
	OamMsg.uiOperType = OAM_CFG_DEL;
	OamMsg.uiRecNum = 4;
	OamMsg.uiSessionId = 5;
	OamMsg.uiTableId = MML_UA_LINK_TABLE_ID;
	OamMsg.usModuleId = 7;
	OamMsg.usNeId = 8;
	OamMsg.usPid = 9;

	OamMsg.pData = (XS8 *)XOS_MemMalloc(FID_UA, sizeof(tPeSmppCfgOamTable));
	pcfgoamtable.index = atoi(ppArgv[1]);


	memcpy(OamMsg.pData, &pcfgoamtable, sizeof(tPeSmppCfgOamTable));
	send_msg = XOS_MsgMemMalloc(FID_UA, sizeof(AGT_OAM_CFG_REQ_T));
	if(XNULL == send_msg)
	{
		XOS_PRINT(MD(FID_UA, PL_ERR), "get thread receive msg:msg malloc failed.");
		return;
	}

	XOS_MemCpy(send_msg->message,&OamMsg,sizeof(AGT_OAM_CFG_REQ_T));

	send_msg->datasrc.PID = XOS_GetLocalPID();
	send_msg->datasrc.FID =  FID_UA;
	send_msg->datadest.PID= XOS_GetLocalPID();
	send_msg->datadest.FID = FID_AGENTOAM;
	send_msg->prio = eNormalMsgPrio;
	if ( XSUCC !=  XOS_MsgSend(send_msg))
	{
		XOS_MsgMemFree( FID_UA, send_msg );
		XOS_PRINT(MD(FID_UA, PL_WARN), "XOS_MsgSend,send message  failed.");
	}
}



/************************************************************************
函数名:  agentUA_GetConfig
功能：   从XML文件得到数据库初始化模块的配置信息
输入：   pFile， xml文件名
输出：   pOut，输出字符串
返回：   成功 XSUCC
         失败 XERROR
说明：
************************************************************************/
XS32 agentUA_GetConfig(XU32 fid, LinkInfo *pOut, XS8* pFile)
{
	xmlDocPtr  doc   = XNULL;
	xmlNodePtr cur      = NULL;
	xmlChar* pTempStr   = NULL;

	if (XNULLP == pOut || XNULLP == pFile)
	{
		XOS_Trace(MD(fid, PL_EXP), "input point is null");
		return XERROR;
	}

	doc =  xmlParseFile( pFile );
	if (doc == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse hlr.xml file failed");
		return XERROR;
	} 
	cur = xmlDocGetRootElement( doc );
	if (cur == XNULL) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse hlr.xml file failed");
		xmlFreeDoc(doc);
		return XERROR;
	}
	if ( XOS_StrCmp( cur->name, "MODULES") ) 
	{
		XOS_Trace(MD(fid, PL_EXP), "parse hlr.xml file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse hlr.xml file failed");
		xmlFreeDoc(doc);
		return( XERROR );
	}
    
	while ( cur != XNULL ) 
	{
		if ( !XOS_StrNcmp(cur->name, "ConnectServerCfg" ,XOS_StrLen( "ConnectServerCfg")))
		{
			break;
		}
		cur = cur->next;
	}
	if ( XNULL == cur )
	{
		XOS_Trace(MD(fid, PL_EXP), "parse hlr.xml file failed");
		xmlFreeDoc(doc);
		return( XERROR ); 
	}
	cur = cur->xmlChildrenNode;
	while ( cur && xmlIsBlankNode ( cur ) )
	{
		cur = cur->next;
	}
	if ( cur == XNULL )
	{
		xmlFreeDoc(doc);
		return( XERROR );
	}

	while(XNULLP != cur)
	{
		if (!XOS_StrCmp(cur->name, "LocalIp"))
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->LocalhostIp, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else if (!XOS_StrCmp(cur->name, "LocalPort"))
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->LocalhostPort, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else if (!XOS_StrCmp(cur->name, "HlrIp"))
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->PeerhostIp, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		else if (!XOS_StrCmp(cur->name, "HlrPort"))
		{
			pTempStr = xmlNodeListGetString( doc, cur->xmlChildrenNode, 1);
			if ( XNULL != pTempStr )
			{
				XOS_StrCpy(pOut->PeerhostPort, pTempStr);
				xmlFree( pTempStr );
				pTempStr = XNULL;
			}
		}
		cur = cur->next;
	}


	xmlFreeDoc(doc);

	return XSUCC;
}

/**************************************************************************
函 数 名: smppUA_InitProc
函数功能: 协议栈初始化函数,对协议栈进行初始化,同时注册OAM回调接口
参    数:
返 回 值: XSUCC 成功  XERROR 失败
**************************************************************************/
XS8 agentUA_InitProc(XVOID *pPara1, XVOID *Para2)
{
	XS32 ret;



    ret = agentUA_GetConfig(FID_UA, &g_hlrLinkInfo, "./hlr.xml");
    if(XERROR == ret)
    {
        agentTrace(XNULL,MD(FID_UA, PL_ERR), "agentUA_GetConfig is Err!");
        return ret;
    }

	/*_SMPP_LINK_LIST_表地创建*/
	gSmppLinkList = XOS_listConstruct(sizeof(tSmppLink), MAX_SMPP_LINK_COUNT, "_SMPP_LINK_LIST_");
	if (XNULL == gSmppLinkList)
	{
		agentTrace(XNULL,MD(FID_UA, PL_ERR), "Construct gSmppLinkList is Err!");
		return XERROR;
	}

	/*_MSG_BUFFER_T表的创建*/
	smppUA_htMsgBufferList = XOS_listConstruct(sizeof(MSG_BUFFER_T), MAX_SMPP_LINK_COUNT, "_MSG_BUFFER_T");
	if (XNULL == smppUA_htMsgBufferList)
	{
		agentTrace(XNULL,MD(FID_UA,PL_ERR), "Construct smppUA_htMsgBufferList is Err!");
		return XERROR;
	}


	ret = XOS_RegistCmdPrompt(SYSTEM_MODE, "agent", "agent调试命令", "无参数" );
    if (ret > 0)
	{
		
		if (0 > XOS_RegistCommand(ret, agentUA_Linkadd, "addLink", "addLink", "无参数" ))
		{
			XOS_PRINT(MD(FID_UA, PL_ERR), "regist addLink command failed"); 
		}
        if (0 > XOS_RegistCommand(ret, agentUA_Linkdelete, "deleteLink", "deleteLink", "无参数" ))
		{
			XOS_PRINT(MD(FID_UA, PL_ERR), "regist deleteLink command failed"); 

		}
        if (0 > XOS_RegistCommand(ret, agentUA_ShowLinkTable, "showLink", "showLink", "无参数" ))
		{
			XOS_PRINT(MD(FID_UA, PL_ERR), "regist showLink command failed"); 
		}

        if (0 > XOS_RegistCommand(ret, agentUA_Test, "test", "test", "无参数" ))
		{
			XOS_PRINT(MD(FID_UA, PL_ERR), "regist showLink command failed"); 
		}
    }

    return XSUCC;
}

/**************************************************************************
函 数 名: smppUA_XosMsgProc
函数功能: XOS平台消息处理函数,用于处理来至XOS平台的消息
参    数:
返 回 值:
**************************************************************************/
XS8 agentUA_XosMsgProc(XVOID *msg, XVOID *pPara)
{
    XS8 ret = XERROR;
    t_XOSCOMMHEAD* pxosMsg = (t_XOSCOMMHEAD*)msg;

	if(XNULL == msg)
    {
        XOS_PRINT(MD(FID_UA,PL_ERR),"Input is NULL!");
		return XERROR;
    }

    switch(pxosMsg->datasrc.FID)
    {
	    case FID_MQTT:
        case FID_HLR:
            /*经过MQTTserver 推送上来的消息 需要发送到hlr网元*/
			ret = agentUA_XosMsgHandler(pxosMsg, pPara);
			break;
		case FID_NTL:
			ret = agentUA_NtlRspProc(pxosMsg);
			break;
		default:
			XOS_PRINT(MD(FID_UA,PL_ERR),"SmppUA recv msg from unknown Fid(%d) msg",pxosMsg->datasrc.FID);
			ret = XERROR;
			break;
    }

    return ret;
}

/**************************************************************************
函 数 名: agentUA_TimeoutProc
函数功能: 用于向XOS平台注册超时回调接口,暂时不需要进行处理
参    数:
返 回 值: XSUCC
**************************************************************************/
XS8 agentUA_TimeoutProc(t_BACKPARA *tBackPara)
{
//	t_XOSCOMMHEAD *pMsgHead;
//	XS32 ret;
	XS32 appHandle;
	XU8  eLinkType;
    t_LINKCLOSEIND tLinkCloseInd = {0};


//	t_BACKPARA tBackParaRsp;
//	t_DATAREQ *ptDataReq;
	tSmppLink* pSmpplink;


    appHandle = tBackPara->para2;
	pSmpplink = (tSmppLink*)XOS_listGetElem(gSmppLinkList, appHandle);
	if (pSmpplink == XNULL)
	{
		agentTrace(XNULL,MD(FID_UA,PL_ERR), "agentUA_TimeoutProc XOS_listGetElem is Err!");
		return XERROR;
	}
	eLinkType = pSmpplink->linkType;

    switch(tBackPara->para1)
    {

        case TIMER_HEART_REQ_SEND :
            //agentUA_HeartSendReq(FID_UA, appHandle);           

            //pSmpplink->usRepeat++;
            break;
        case TIMER_HEART_WATCHER :
            #if 0
    		if (eLinkType != eTCPClient)
    		{
    			agentTrace(XNULL,MD(FID_UA, PL_ERR), "agentUA_TimeoutProc eLinkType is not tcpclient !");
    			return XERROR;
    		}      

    		if (pSmpplink->usRepeat > MAX_HEART_BEAT_TIMES)
    		{
                //pSmpplink->linkStatus  = LINKSTATUS_ERROR;
                agentTrace(XNULL,MD(FID_UA, PL_ERR), "pSmpplink->linkStatus is error");

    			/*模拟NTL发送tStopInd消息*/
    			pMsgHead=XOS_MsgMemMalloc(FID_UA, sizeof(t_LINKCLOSEIND));
    			if (XNULLP == pMsgHead)
    			{
    				agentTrace(XNULL,MD(FID_UA,PL_ERR), "smppUA_TimeoutProc MsgMemMalloc is Err!");
    				return XERROR;
    			}
    			pMsgHead->datasrc.FID =  FID_UA;
    			pMsgHead->datadest.FID = FID_NTL;
    			pMsgHead->msgID = eStopInd;
    			pMsgHead->prio = eNormalMsgPrio;

    			tLinkCloseInd.appHandle = (HAPPUSER)appHandle;
    			tLinkCloseInd.peerAddr.ip = ntohl(inet_addr((char*)pSmpplink->peerMasterIp));
    			tLinkCloseInd.peerAddr.port = pSmpplink->peerPort;
    			tLinkCloseInd.closeReason = eOtherCloseReason;
    			XOS_MemCpy(pMsgHead->message, &tLinkCloseInd, sizeof(t_LINKCLOSEIND));

    			ret=XOS_MsgSend(pMsgHead);
    			if (XERROR == ret)
    			{
    				/*释放掉内存空间*/                
    				XOS_MemFree(FID_UA, pMsgHead);
    				agentTrace(XNULL,MD(FID_UA, PL_ERR), "agentUA_TimeoutProc MsgSend is Err!");
    				return XERROR;
    			}  

    			
            }
            
            break;
           #endif
        default:
            break;

    }

	return XSUCC;
}

XS8 agentUA_NoticeProc(XVOID* pLVoid, XVOID* pRVoid)
{

	t_XOSCOMMHEAD *send_msg =XNULL;
	XS32 size=0;
	AGT_OAM_CFG_REQ_T OamMsg ={0};
	tPeSmppCfgOamTable pcfgoamtable = {0};

    if(0 == XOS_MemCmp(g_hlrLinkInfo.PeerhostIp, "0.0.0.0", XOS_StrLen("0.0.0.0")))
    {
        XOS_PRINT(MD(FID_UA, PL_ERR), "HLR ip is invalied.");
        return  XSUCC;
    }

	OamMsg.uiIndex = 1;
	OamMsg.uiMsgLen = 2;
	OamMsg.uiOperType = OAM_CFG_ADD;
	OamMsg.uiRecNum = 4;
	OamMsg.uiSessionId = 5;
	OamMsg.uiTableId = MML_UA_LINK_TABLE_ID;
	OamMsg.usModuleId = 7;
	OamMsg.usNeId = 8;
	OamMsg.usPid = 9;

	OamMsg.pData = (XS8 *)XOS_MemMalloc(FID_UA, sizeof(tPeSmppCfgOamTable));
	pcfgoamtable.index = 1;
    pcfgoamtable.linkType = 2;//tcpclient
	XOS_StrCpy(pcfgoamtable.smcIp, g_hlrLinkInfo.LocalhostIp);
	pcfgoamtable.smcPort = atoi((const char *)g_hlrLinkInfo.LocalhostPort);

	XOS_StrCpy(pcfgoamtable.peerIp , g_hlrLinkInfo.PeerhostIp);
	pcfgoamtable.peerPort = atoi((const char *)g_hlrLinkInfo.PeerhostPort);

	XOS_StrCpy(pcfgoamtable.linkDesc, "Link to hlr");
	pcfgoamtable.linkStatus = LINKSTATUS_ERROR;

	memcpy(OamMsg.pData, &pcfgoamtable, sizeof(tPeSmppCfgOamTable));
	send_msg = XOS_MsgMemMalloc(FID_UA, sizeof(AGT_OAM_CFG_REQ_T));
	if(XNULL == send_msg)
	{
		XOS_PRINT(MD(FID_UA, PL_ERR), "get thread receive msg:msg malloc failed.");
		return XERROR;
	}

	XOS_MemCpy(send_msg->message,&OamMsg,sizeof(AGT_OAM_CFG_REQ_T));

	send_msg->datasrc.PID = XOS_GetLocalPID();
	send_msg->datasrc.FID =  FID_UA;
	send_msg->datadest.PID= XOS_GetLocalPID();
	send_msg->datadest.FID = FID_AGENTOAM;
	send_msg->prio = eNormalMsgPrio;
	if ( XSUCC !=  XOS_MsgSend(send_msg))
	{
		XOS_MsgMemFree( FID_UA, send_msg );
		XOS_PRINT(MD(FID_UA, PL_WARN), "XOS_MsgSend,send message  failed.");
	}
	return XSUCC;
}




