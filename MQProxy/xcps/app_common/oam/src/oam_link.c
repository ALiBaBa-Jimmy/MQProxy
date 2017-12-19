/*******************************************************************************
* Copyright(C): Xinwei Telecom Technology Inc
* FileName:     oam_link.cpp
* Author:       weizhao
* Date：        2014-09-26
* OverView:     oam link managemnet
*
* History:      create
* Revisor:      
* Date:         2014-09-26
* Description:  create the file
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

#include "oam_link.h"

OAM_LINK_CB_T g_OamLinkCB;
XEXTERN XS8 g_ProcessType[VALUE_MAX_LEN];
XEXTERN OAM_TID_MID_T g_AsyncTidMidArray[MAX_TB_NUM];
XEXTERN XU32 g_AsyncTidMidCnt;
PTIMER  g_AsyncTmHandle;      //定时器句柄

/*****************************************************************************
*函数名:OAM_LinkCBInit
*功能  :OAM链路控制块初始化
*输入  :
*输出  :
*返回  :
*           XVOID
*说明  :
*****************************************************************************/
XVOID OAM_LinkCBInit(OAM_LINK_CFG_T oamCfg)
{
    XU32 uiLoop = 0;
    XCHAR ucIpStr[MAX_LINK_NUM][MAX_IPSTR_LEN+1] = { {0}, {0} };
    
#ifdef WIN32
    t_LOCALIPLIST stLocalIPList = {0};
    XOS_GetLocalIP(&stLocalIPList);
    stLocalIPList.localIP[0] = XOS_NtoHl(stLocalIPList.localIP[0]);
#endif

    XOS_MemSet(&g_OamLinkCB, 0, sizeof(OAM_LINK_CB_T)); 
    
    XOS_INIT_THDLE(g_AsyncTmHandle);

    g_OamLinkCB.uiAppHandle = OAM_LINK_APPHANDLE;
    
    g_OamLinkCB.usNeId    = (XU16)oamCfg.siNeID;
    g_OamLinkCB.usPid     = (XU16)oamCfg.siPID;
    g_OamLinkCB.siFrameId = (XS32)oamCfg.siFrameId;
    g_OamLinkCB.siSlotId  = (XS32)oamCfg.siSlotId;

#ifdef WIN32
    g_OamLinkCB.uiPeerIp[0] = stLocalIPList.localIP[0];
    g_OamLinkCB.uiPeerIp[1] = stLocalIPList.localIP[0];
#else
    OAM_LinkAgtBaseIpAddrGet(oamCfg.peerIp, &g_OamLinkCB.uiPeerIp[0], &g_OamLinkCB.uiPeerIp[1]);
#endif
    g_OamLinkCB.usPeerPort = AGT_PORT;

    /*根据规则计算端口*/
    if (!XOS_StrNcmp(g_ProcessType, "bm", XOS_StrLen("bm"))) 
    {
        g_OamLinkCB.usPort = OAM_BM_PORT;
    }
    else if(!XOS_StrNcmp(g_ProcessType, "ts", XOS_StrLen("ts")))
    {
        g_OamLinkCB.usPort = OAM_TS_PORT;
    }
    else
    {
        g_OamLinkCB.usPort = NE_PORT_BASE + (g_OamLinkCB.usNeId<<NE_PORT_NEID_OFFSET) 
                                          + (g_OamLinkCB.usPid<<NE_PORT_PID_OFFSET);
    }
    
    for(uiLoop=0; uiLoop<MAX_LINK_NUM; uiLoop++)
    {
        /*框号从1开始, ip从0开始:siFrameId - 1*/
        if(g_OamLinkCB.siFrameId > 1)
        {
            XOS_Sprintf(ucIpStr[uiLoop], MAX_IPSTR_LEN, "172.16.%d%d.%d", 
                g_OamLinkCB.siFrameId - 1, uiLoop, g_OamLinkCB.siSlotId);
        }
        else/*g_OamLinkCB.siFrameId == 1*/
        {
            XOS_Sprintf(ucIpStr[uiLoop], MAX_IPSTR_LEN, "172.16.%d.%d", 
                                uiLoop, g_OamLinkCB.siSlotId);
        }

#ifdef WIN32
        g_OamLinkCB.uiIpAddr[uiLoop] = stLocalIPList.localIP[0];
#else
        XOS_StrtoIp(ucIpStr[uiLoop], &g_OamLinkCB.uiIpAddr[uiLoop]);
#endif
    }
}

/*****************************************************************************
*函数名:OAM_LinkInit
*功能  :链路初始化
*输入  :
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS8 OAM_LinkInit()
{
    t_XOSCOMMHEAD* pLinkInit = XNULL;
    t_LINKINIT* pLinkHead    = XNULL;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkInit\r\n");

    pLinkInit = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_LINKINIT));
    if(XNULL == pLinkInit)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "malloc LinkInit msg mem failed\r\n");
        return XERROR;
    }

    SET_XOS_MSG_HEAD(pLinkInit, eLinkInit, FID_OAM, FID_NTL);

    pLinkHead = (t_LINKINIT*)pLinkInit->message;
    pLinkHead->linkType = eSCTPClient;
    pLinkHead->ctrlFlag = eNullLinkCtrl;
    pLinkHead->appHandle = (HAPPUSER)(XPOINT)g_OamLinkCB.uiAppHandle;

    if(XSUCC != XOS_MsgSend(pLinkInit))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        XOS_MsgMemFree(FID_OAM, pLinkInit);
        return XERROR;
    }

    OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
        LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkInitAckProc
*功能  :链路 Init Ack 处理
*输入  :
*           t_XOSCOMMHEAD* pMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkInitAckProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKINITACK* pLinkInitAck = XNULL;
    XU32 uiAppHandle = 0;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkInitAckProc\r\n");

    if(XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkInitAckProc: pMsg is empty\r\n");
        return XERROR;
    }

    pLinkInitAck = (t_LINKINITACK*)pMsg->message;
    if(XNULL == pLinkInitAck)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link init ack, message is empty\r\n");
        return XERROR;
    }

    uiAppHandle = (XU32)(XPOINT)pLinkInitAck->appHandle;
    if(g_OamLinkCB.uiAppHandle != uiAppHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link init ack, appHandle[%d] is not match [%d]\r\n", 
                                                             uiAppHandle, g_OamLinkCB.uiAppHandle);
        return XERROR;
    }

    OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);

    if(eSUCC != pLinkInitAck->lnitAckResult)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link init ack, result[%d] is not eSucc\r\n", 
                                                                  pLinkInitAck->lnitAckResult);

        /*设置定时器重新初始化链路*/
        OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
            LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);

        return XERROR;
    }

    /*记录底层物理链路索引*/
    g_OamLinkCB.hLinkHandle = pLinkInitAck->linkHandle;

    /*启动链路*/
    return OAM_LinkStart();
}

/*****************************************************************************
*函数名:OAM_LinkStart
*功能  :链路启动
*输入  :
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkStart()
{
    t_XOSCOMMHEAD* pLinkStart = XNULL;
    t_LINKSTART* pLinkHead    = XNULL;
    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkStart\r\n");

    pLinkStart = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_LINKSTART));
    if(XNULL == pLinkStart)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "malloc LinkStart msg mem failed\r\n");
        return XERROR;
    }

    SET_XOS_MSG_HEAD(pLinkStart, eLinkStart, FID_OAM, FID_NTL);

    pLinkHead = (t_LINKSTART*)pLinkStart->message;
    pLinkHead->linkHandle = g_OamLinkCB.hLinkHandle;

    pLinkHead->linkStart.sctpClientStart.myAddr.ip[0] = g_OamLinkCB.uiIpAddr[0];
    pLinkHead->linkStart.sctpClientStart.myAddr.ip[1] = g_OamLinkCB.uiIpAddr[1];
    pLinkHead->linkStart.sctpClientStart.myAddr.port = g_OamLinkCB.usPort;
#ifdef WIN32
    pLinkHead->linkStart.sctpClientStart.myAddr.ipNum = 1;
#else
    pLinkHead->linkStart.sctpClientStart.myAddr.ipNum = MAX_LINK_NUM;
#endif

    pLinkHead->linkStart.sctpClientStart.peerAddr.ip[0] = g_OamLinkCB.uiPeerIp[0];
    pLinkHead->linkStart.sctpClientStart.peerAddr.ip[1] = g_OamLinkCB.uiPeerIp[1];
    pLinkHead->linkStart.sctpClientStart.peerAddr.port = g_OamLinkCB.usPeerPort;
#ifdef WIN32
    pLinkHead->linkStart.sctpClientStart.peerAddr.ipNum = 1;
#else
    pLinkHead->linkStart.sctpClientStart.peerAddr.ipNum = MAX_LINK_NUM;
#endif

    pLinkHead->linkStart.sctpClientStart.hbInterval = 0;
    pLinkHead->linkStart.sctpClientStart.pathmaxrxt = 0;
    pLinkHead->linkStart.sctpClientStart.streamNum = 0;

    OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
        LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKSTART, 0, 0, 0);

    if(XSUCC != XOS_MsgSend(pLinkStart))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        XOS_MsgMemFree(FID_OAM, pLinkStart);
        return XERROR;
    }

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkStartAckProc
*功能  :链路 Start Ack 处理
*输入  :
*           t_XOSCOMMHEAD* pMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkStartAckProc(t_XOSCOMMHEAD* pMsg)
{
    t_SCTPSTARTACK* pSctpLinkStartAck = XNULL;
    XU32 uiAppHandle = 0;
    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkStartAckProc\r\n");

    if(XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkStartAckProc: pMsg is null\r\n");
        return XERROR;
    }

    pSctpLinkStartAck = (t_SCTPSTARTACK*)pMsg->message;
    if(XNULL == pSctpLinkStartAck)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link start ack, message is empty\r\n");
        return XERROR;
    }

    uiAppHandle = (XU32)(XU64)pSctpLinkStartAck->appHandle;
    if(g_OamLinkCB.uiAppHandle != uiAppHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link start ack, appHandle[%d] is not match[%d]\r\n",
                                                             uiAppHandle, g_OamLinkCB.uiAppHandle);
        return XERROR;
    }

    OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);

    switch(pSctpLinkStartAck->linkStartResult)
    {
    case eSUCC:
        break;

    case eBlockWait:
        XOS_PRINT(MD(FID_OAM, PL_LOG), "link start ack, result is eBlockWait\r\n");
        return XSUCC;

    default:/*failed*/
        {
            XOS_PRINT(MD(FID_OAM, PL_DBG), "ERROR: link start ack, result[%d] is not eSUCC\r\n",
                                                              pSctpLinkStartAck->linkStartResult);

            /*释放链路*/
            OAM_LinkRelease(uiAppHandle);

            /*设置定时器重新初始化链路*/
            OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
                LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);

            return XERROR;
        }
    }
    
    return OAM_LinkRegSend();
}


/*****************************************************************************
*函数名:OAM_LinkDataIndProc
*功能  :链路 DataInd 处理
*输入  :
*           t_XOSCOMMHEAD* pMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkDataIndProc(t_XOSCOMMHEAD* pMsg)
{
    t_SCTPDATAIND* pSctpDataInd = XNULL;
    XU32 uiAppHandle  = 0;
    XU32 uiMsgType      = 0;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkDataIndProc\r\n");

    if(XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkDataIndProc: pMsg is null\r\n");
        return XERROR;
    }

    pSctpDataInd = (t_SCTPDATAIND*)pMsg->message;
    if(XNULL == pSctpDataInd)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link DataInd, message is empty\r\n");
        return XERROR;
    }

    if(XNULL == pSctpDataInd->pData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link DataInd, message pData is empty\r\n");
        return XERROR;
    }

    /*获取链路控制块索引*/
    uiAppHandle = (XU32)(XU64)pSctpDataInd->appHandle;
    if(g_OamLinkCB.uiAppHandle != uiAppHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link DataInd, appHandle[%d] is not match [%d]\r\n",
                                                             uiAppHandle, g_OamLinkCB.uiAppHandle);
        return XERROR;
    }

    /*if(   ( pSctpDataInd->peerAddr.ip != g_OamLinkCB.uiPeerIp[0] 
    && pSctpDataInd->peerAddr.ip != g_OamLinkCB.uiPeerIp[1]) 
    || pSctpDataInd->peerAddr.port != g_OamLinkCB.usPeerPort)
    {
    XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRegAckProc: invalid reg ack\r\n");
    return XERROR;
    }*/

    /*前4个byte是msgType*/
    XOS_MemCpy(&uiMsgType, pSctpDataInd->pData, sizeof(XU32));
    uiMsgType = XOS_NtoHl(uiMsgType);

    switch(uiMsgType)
    {
    case LINK_MSG_REG_ACK:    /*处理 REG_ACK 消息*/
        if(XSUCC != OAM_LinkRegAckProc((XVOID*)pSctpDataInd))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_LinkRegAckProc failed!\r\n");
            return XERROR;
        }
        break;

    case LINK_MSG_HS:         /*处理 HS 消息*/
        if(XSUCC != OAM_LinkHsProc((XVOID*)pSctpDataInd))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_LinkHsProc failed!\r\n");
            return XERROR;
        }
        break;

    case LINK_MSG_HS_ACK:     /*处理 HS_ACK 消息*/
        if(XSUCC != OAM_LinkHsAckProc((XVOID*)pSctpDataInd))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_LinkHsAckProc failed!\r\n");
            return XERROR;
        }
        break;

    case AGENT_CFG_MSG:       /*配置消息*/
        if(XSUCC != OAM_AgtCfgMsgProc((XVOID*)&pSctpDataInd->pData[sizeof(XU32)]))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtCfgMsgProc failed!\r\n");
            return XERROR;
        }
        break;

    case AGENT_SYNC_MSG:       /*AGENT同步消息*/
        if(XSUCC != OAM_AgtDataMsgProc((XVOID*)&pSctpDataInd->pData[sizeof(XU32)], AGENT_CFG_MSG))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtDataMsgProc failed!\r\n");
            return XERROR;
        }
        break;
        
    case OAM_MSG_NAV_INFO:     /*导航树信息*/
        if(XSUCC != OAM_TsNavMsgProc((XVOID*)&pSctpDataInd->pData[sizeof(XU32)]))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_TsNavMsgProc failed!\r\n");
            return XERROR;
        }
        break;
        
    case MSG_CMD_PROC_OPRE:       /*启动/停止/复位等进程消息*/
        if(XSUCC != OAM_TransMsgSend((XVOID*)&pSctpDataInd->pData[sizeof(XU32)], uiMsgType, FID_BM))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_TransMsgSend failed!\r\n");
            return XERROR;
        }
        break;
        
    case MSG_REQ_PROC_OPER: /*BM请求启动进程列表信息*/ 
        if(XSUCC != OAM_OperMsgSend(FID_BM, uiMsgType, pSctpDataInd->dataLenth - sizeof(XU32),
                                                      (XVOID*)&pSctpDataInd->pData[sizeof(XU32)]))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_OperMsgSend failed!\r\n");
            return XERROR;
        }
        break;
        
    case OAM_MSG_IP_RSP: /*TS请求IP和主备槽位信息*/ 
        if(XSUCC != OAM_OperMsgSend(FID_TS, uiMsgType, pSctpDataInd->dataLenth - sizeof(XU32),
                                                        (XVOID*)&pSctpDataInd->pData[sizeof(XU32)]))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_OperMsgSend failed!\r\n");
            return XERROR;
        }
        break;
        
    case MSG_CMD_POWERDONW:       /*单板下电消息*/
    case MSG_CMD_BOARD_RESET:     /*重启单板消息*/
    case MSG_CMD_BOARD_DEL:       /*删除板消息*/
        if(XSUCC != OAM_MsgIdSend(FID_BM, uiMsgType))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_MsgIdSend failed!\r\n");
            return XERROR;
        }
        break;
    case AGENT_FMT_MSG:
        if(XSUCC != OAM_AgtFmtMsgProc())
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtFmtMsgProc failed!\r\n");
            return XERROR;
        }
        break;
    case AGENT_DEBUG_REG_MSG:/*软调消息*/
        if(XSUCC != OAM_AgtSftDbgMsgProc((XVOID*)&pSctpDataInd->pData[sizeof(XU32)]))
        {
            PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);
            XOS_PRINT(MD(FID_OAM,PL_ERR),"OAM_AgtSftDbgMsgProc failed!\r\n");
            return XERROR;
        }
        break;
    default:
        XOS_PRINT(MD(FID_OAM, PL_WARN), "Unknown DataInd message to FID_OAM\r\n");
        break;
    }

    PTR_MEM_FREE(FID_OAM, pSctpDataInd->pData);

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkStopIndProc
*功能  :链路 StopInd 处理
*输入  :
*           t_XOSCOMMHEAD* pMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkStopIndProc(t_XOSCOMMHEAD* pMsg)
{
    t_LINKCLOSEIND* pStopInd = XNULL;
    XU32 uiAppHandle = 0;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkStopIndProc\r\n");

    if(XNULL == pMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkStopIndProc: pMsg is null\r\n");
        return XERROR;
    }

    pStopInd = (t_LINKCLOSEIND*)(pMsg + 1);
    if(XNULL == pStopInd)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link StopInd, message is empty\r\n");
        return XERROR;
    }

    /*获取链路控制块索引*/
    uiAppHandle = (XU32)(XU64)pStopInd->appHandle;
    if(g_OamLinkCB.uiAppHandle != uiAppHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: link StopInd, appHandle is not match\r\n");
        return XERROR;
    }

    XOS_PRINT(MD(FID_OAM, PL_WARN), "oam link close peerAddr(%d, %d), close reason(%d)\r\n", 
        pStopInd->peerAddr.ip, pStopInd->peerAddr.port, pStopInd->closeReason);

    /*关闭定时器*/
    OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);

    /*释放链路*/
    OAM_LinkRelease(uiAppHandle);

    /*设置定时器重新初始化链路*/
    OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
        LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);

    g_OamLinkCB.uiState = LINK_STATE_NULL;

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkRelease
*功能  :释放链路
*输入  :
*           XU32 uiAppHandle
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkRelease(XU32 uiAppHandle)
{
    t_XOSCOMMHEAD* pLinkRelease     = XNULL;
    t_LINKRELEASE* pLinkReleaseHead = XNULL;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkRelease\r\n");
    
    if(g_OamLinkCB.uiAppHandle != uiAppHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRelease: uiAppHandle[%d] is not match [%d]\r\n",
                                                            uiAppHandle, g_OamLinkCB.uiAppHandle);
        return XERROR;
    }

    pLinkRelease = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_LINKRELEASE));
    if(XNULL == pLinkRelease)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "malloc LinkReleaseMsg mem fail\r\n");
        return XERROR;
    }

    SET_XOS_MSG_HEAD(pLinkRelease, eLinkRelease, FID_OAM, FID_NTL);

    pLinkReleaseHead = (t_LINKRELEASE*)pLinkRelease->message;
    pLinkReleaseHead->linkHandle = g_OamLinkCB.hLinkHandle;

    if(XSUCC != XOS_MsgSend(pLinkRelease))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        XOS_MsgMemFree(FID_OAM, pLinkRelease);
        return XERROR;
    }

    g_OamLinkCB.uiState = LINK_STATE_NULL;

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkRegSend
*功能  :发送注册请求
*输入  :
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS8 OAM_LinkRegSend()
{
    t_XOSCOMMHEAD* pRegMsg = XNULL;
    XCHAR* pData = XNULL;
    LINK_REG_T* pRegData = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XU32 msgLenth = 0;
    XU32 uiMsgType = 0;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkRegSend\r\n");

    msgLenth = sizeof(LINK_REG_T) + sizeof(XU32);
    pData = (XCHAR*)XOS_MemMalloc(FID_OAM, msgLenth);
    if(XNULL == pData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRegSend: REG mem malloc failed\r\n");
        return XERROR;
    }

    pRegMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
    if(XNULL == pRegMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRegSend: REG msg mem malloc failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        return XERROR;
    }
    XOS_MemSet(pRegMsg->message, 0, sizeof(t_SCTPDATAREQ));

    SET_XOS_MSG_HEAD(pRegMsg, eSendData, FID_OAM, FID_NTL);

    g_OamLinkCB.uiRegSeq++;
    uiMsgType = XOS_HtoNl(LINK_MSG_REG);
    XOS_MemCpy(pData, &uiMsgType, sizeof(XU32));
    pRegData = (LINK_REG_T*)&pData[sizeof(XU32)];
    pRegData->uiRegSeq = XOS_HtoNl(g_OamLinkCB.uiRegSeq);
    pRegData->uiNeID = XOS_HtoNl((XU32)g_OamLinkCB.usNeId);
    pRegData->uiPID = XOS_HtoNl((XU32)g_OamLinkCB.usPid);
    pRegData->uiFrameId = XOS_HtoNl((XU32)g_OamLinkCB.siFrameId);
    pRegData->uiSlotId = XOS_HtoNl((XU32)g_OamLinkCB.siSlotId);

    pNtlHead  = (t_SCTPDATAREQ*)pRegMsg->message;
    pNtlHead->linkHandle = g_OamLinkCB.hLinkHandle;
    pNtlHead->msgLenth = msgLenth;
    pNtlHead->pData = pData;

    //启动定时器
    OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
        LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_REG, 0, 0, 0);

    if(XSUCC != XOS_MsgSend(pRegMsg))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        XOS_MsgMemFree(FID_OAM, pRegMsg);
        return XERROR;
    }

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkRegAckProc
*功能  :注册回应处理
*输入  :
*           XVOID* pRegAckMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkRegAckProc(XVOID* pRegAckMsg)
{
    t_SCTPDATAIND* pSctpDataInd = XNULL;
    LINK_REGACK_T  stRegAckData = {0};

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkRegAckProc\r\n");

    if(XNULL == pRegAckMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "pRegAckMsg is null\r\n");
        return XERROR;
    }

    pSctpDataInd = (t_SCTPDATAIND*)pRegAckMsg;

    /*前4个byte是msgType, 后面跟着LINK_REGACK_T*/
    if(sizeof(XU32) + sizeof(LINK_REGACK_T) != pSctpDataInd->dataLenth)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRegAckProc: invalid dataLenth[%d]\r\n",
                                                                pSctpDataInd->dataLenth);
        return XERROR;
    }
    XOS_MemCpy(&stRegAckData, &pSctpDataInd->pData[sizeof(XU32)], sizeof(LINK_REGACK_T));
    stRegAckData.uiRegAckSeq = XOS_NtoHl(stRegAckData.uiRegAckSeq);

    /*校验注册回应序列号
    if(stRegAckData.uiRegAckSeq != g_OamLinkCB.uiRegSeq)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkRegAckProc: invalid reg ack seq\r\n");
        return XERROR;
    }*/

    OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);
    g_OamLinkCB.uiTimeOutCnt = 0;
    g_OamLinkCB.uiState = LINK_STATE_CONNECT;

    /*启动握手定时器*/
    OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
        LINK_HS_TIMEOUT_LEN, TIMER_TYPE_LOOP, TM_TYPE_HS, 0, 0, 0);

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkHsSend
*功能  :发送握手请求
*输入  :
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS8 OAM_LinkHsSend()
{
    t_XOSCOMMHEAD* pHsMsg = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XCHAR* pData = XNULL;
    LINK_HS_T* pHsData = XNULL;
    XU32 msgLenth = 0;
    XU32 uiMsgType = 0;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkHsSend\r\n");

    /*判断链路状态*/
    if(LINK_STATE_CONNECT != g_OamLinkCB.uiState 
        && LINK_STATE_DISCONNECT != g_OamLinkCB.uiState)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "ERROR: HS send, link state[%d] is not CONNECT\r\n",
                                                                      g_OamLinkCB.uiState);
        return XERROR;
    }

    msgLenth = sizeof(LINK_HS_T) + sizeof(XU32);
    pData = (XCHAR*)XOS_MemMalloc(FID_OAM, msgLenth);
    if(XNULL == pData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsSend: HS mem malloc failed\r\n");
        return XERROR;
    }

    pHsMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
    if(XNULL == pHsMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsSend: HS msg mem malloc failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        return XERROR;
    }
    XOS_MemSet(pHsMsg->message, 0, sizeof(t_SCTPDATAREQ));

    SET_XOS_MSG_HEAD(pHsMsg, eSendData, FID_OAM, FID_NTL);

    g_OamLinkCB.uiHsSeq++;
    uiMsgType = XOS_HtoNl(LINK_MSG_HS);
    XOS_MemCpy(pData, &uiMsgType, sizeof(XU32));
    pHsData = (LINK_HS_T*)&pData[sizeof(XU32)];
    pHsData->uiHsSeq = XOS_HtoNl(g_OamLinkCB.uiHsSeq);
    pHsData->uiNeID = XOS_HtoNl((XU32)g_OamLinkCB.usNeId);
    pHsData->uiPID = XOS_HtoNl((XU32)g_OamLinkCB.usPid);
    pHsData->uiFrameId = XOS_HtoNl((XU32)g_OamLinkCB.siFrameId);
    pHsData->uiSlotId = XOS_HtoNl((XU32)g_OamLinkCB.siSlotId);

    pNtlHead  = (t_SCTPDATAREQ*)pHsMsg->message;
    pNtlHead->linkHandle = g_OamLinkCB.hLinkHandle;
    pNtlHead->msgLenth = msgLenth;
    pNtlHead->pData = pData;

    if(XSUCC != XOS_MsgSend(pHsMsg))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        XOS_MsgMemFree(FID_OAM, pHsMsg);
        return XERROR;
    }

    XOS_PRINT(MD(FID_OAM, PL_DBG),"start hs ack timer\r\n");

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkHsProc
*功能  :握手请求处理
*输入  :
*           XVOID* pHsMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkHsProc(XVOID* pHsMsg)
{
    t_SCTPDATAIND* pSctpDataInd = XNULL;

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkHsProc\r\n");

    if(XNULL == pHsMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "pHsMsg is null\r\n");
        return XERROR;
    }   
    
    pSctpDataInd = (t_SCTPDATAIND*)(pHsMsg);

    if(sizeof(XU32) + sizeof(LINK_HS_T) != pSctpDataInd->dataLenth)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsProc: invalid dataLenth[%d]\r\n",
                                                                pSctpDataInd->dataLenth);
        return XERROR;
    }

    /*收到HS,修改链路状态为连接*/
    if(LINK_STATE_CONNECT != g_OamLinkCB.uiState)
    {
        g_OamLinkCB.uiState = LINK_STATE_CONNECT;
    }

    g_OamLinkCB.uiTimeOutCnt = 0;

    return OAM_LinkHsAckSend(pHsMsg);
}

/*****************************************************************************
*函数名:OAM_LinkHsAckSend
*功能  :发送握手回应
*输入  :
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkHsAckSend(XVOID* pHsMsg)
{
    t_SCTPDATAIND* pSctpDataInd = XNULL;
    t_XOSCOMMHEAD* pHsAckMsg = XNULL;
    t_SCTPDATAREQ* pNtlHead = XNULL;
    XCHAR* pData = XNULL;
    XU32 msgLenth = 0;

    LINK_HSACK_T* pHsAckData = XNULL;
    LINK_HS_T stHsData = {0};

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkHsAckSend\r\n");

    if(XNULL == pHsMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckSend: hs msg is null\r\n");
        return XERROR;
    }

    msgLenth = sizeof(LINK_HSACK_T) + sizeof(XU32);
    pData = (XCHAR*)XOS_MemMalloc(FID_OAM, msgLenth);
    if(XNULL == pData)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckSend: HsAck mem malloc failed\r\n");
        return XERROR;
    }

    pHsAckMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_OAM, sizeof(t_SCTPDATAREQ));
    if(XNULL == pHsAckMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckSend: HsAck msg mem malloc failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        return XERROR;
    }
    XOS_MemSet(pHsAckMsg->message, 0, sizeof(t_SCTPDATAREQ));

    SET_XOS_MSG_HEAD(pHsAckMsg, eSendData, FID_OAM, FID_NTL);

    pSctpDataInd = (t_SCTPDATAIND*)(pHsMsg);

    /*前4个byte是msgType, 后面跟着LINK_HS_T*/
    XOS_MemCpy(&stHsData, &pSctpDataInd->pData[sizeof(XU32)], sizeof(LINK_HS_T*));
    stHsData.uiHsSeq = XOS_NtoHl(stHsData.uiHsSeq);

    /*msgType*/
    *(XU32*)pData = XOS_HtoNl(LINK_MSG_HS_ACK);

    /*LINK_HSACK_T*/
    pHsAckData = (LINK_HSACK_T*)&pData[sizeof(XU32)];
    XOS_PRINT(MD(FID_OAM, PL_DBG),"stHsData.uiHsSeq = %d\r\n", stHsData.uiHsSeq);
    pHsAckData->uiHsAckSeq = XOS_HtoNl(stHsData.uiHsSeq);

    pNtlHead  = (t_SCTPDATAREQ*)pHsAckMsg->message;
    pNtlHead->linkHandle = g_OamLinkCB.hLinkHandle;
    pNtlHead->msgLenth = msgLenth;
    pNtlHead->pData = pData;

    if(XSUCC != XOS_MsgSend(pHsAckMsg))
    {
        XOS_PRINT(MD(FID_OAM,PL_ERR),"XOS_MsgSend failed\r\n");
        PTR_MEM_FREE(FID_OAM, pData);
        XOS_MsgMemFree(FID_OAM, pHsAckMsg);
        return XERROR;
    }

    return XSUCC;
}

/*****************************************************************************
*函数名:OAM_LinkHsAckProc
*功能  :握手回应处理
*输入  :
*           XVOID* pHsAckMsg
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkHsAckProc(XVOID* pHsAckMsg)
{
    t_SCTPDATAIND* pSctpDataInd = XNULL;
    LINK_HSACK_T  stHsAckData = {0};

    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkHsAckProc\r\n");

    if(XNULL == pHsAckMsg)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckProc: pHsAckMsg is null\r\n");
        return XERROR;
    }

    pSctpDataInd = (t_SCTPDATAIND*)pHsAckMsg;

    if(sizeof(XU32) + sizeof(LINK_HSACK_T) != pSctpDataInd->dataLenth)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckProc: invalid dataLenth[%d]\r\n",
                                                                pSctpDataInd->dataLenth);
        return XERROR;
    }

    /*前4个byte是msgType, 后面跟着LINK_HSACK_T*/
    XOS_MemCpy(&stHsAckData, &pSctpDataInd->pData[sizeof(XU32)], sizeof(LINK_HSACK_T));
    stHsAckData.uiHsAckSeq = XOS_NtoHl(stHsAckData.uiHsAckSeq);

    /*校验握手回应序列号*/
    if(stHsAckData.uiHsAckSeq != g_OamLinkCB.uiHsSeq)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkHsAckProc: invalid hs ack seq\r\n");
        return XERROR;
    }

    g_OamLinkCB.uiTimeOutCnt = 0;

    return XSUCC;
}


/*****************************************************************************
*函数名:OAM_LinkTimeOutProc
*功能  :定时器超时处理
*输入  :
*           PTIMER* tHandle
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS8 OAM_LinkTimeOutProc(t_BACKPARA* para)
{
    XS8 siRet = XSUCC;
    TM_TYPE_E eTmType = TM_TYPE_NULL;
    XU32 uiLoop = 0;
    OAM_LINK_T stOamLink = { {0} };
    
    if(XNULL == para)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkTimeOutProc: para is null");
        return XERROR;
    }

    eTmType = (TM_TYPE_E)para->para1;
    XOS_PRINT(MD(FID_OAM, PL_DBG), "eTmType = %d\r\n", eTmType);

    switch(eTmType)
    {
    case TM_TYPE_LINKINIT:
        siRet = OAM_LinkInit();
        break;
    case TM_TYPE_LINKSTART:
        /*释放链路*/
        OAM_LinkRelease(g_OamLinkCB.uiAppHandle);
        /*设置定时器重新初始化链路*/
        OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
            LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);
        break;

    case TM_TYPE_REG:
        g_OamLinkCB.uiTimeOutCnt++;
        if(g_OamLinkCB.uiTimeOutCnt >= MAX_RETRY_NUM)
        {
            OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);
            g_OamLinkCB.uiState = LINK_STATE_DISCONNECT;
            g_OamLinkCB.uiTimeOutCnt = 0;

            /*释放链路*/
            OAM_LinkRelease(g_OamLinkCB.uiAppHandle);
            /*设置定时器重新初始化链路*/
            OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
                LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);
        }
        else
        {
            siRet = OAM_LinkRegSend();
        }
        break;

    case TM_TYPE_HS:
        g_OamLinkCB.uiTimeOutCnt++;
        if(g_OamLinkCB.uiTimeOutCnt >= MAX_RETRY_NUM)
        {
            OAM_LinkTimerStop(&g_OamLinkCB.pTmHandle);
            g_OamLinkCB.uiState = LINK_STATE_DISCONNECT;
            g_OamLinkCB.uiTimeOutCnt = 0;

            /*释放链路*/
            OAM_LinkRelease(g_OamLinkCB.uiAppHandle);
            /*设置定时器重新初始化链路*/
            OAM_LinkTimerStart(&g_OamLinkCB.pTmHandle, 
                LINK_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_LINKINIT, 0, 0, 0);
        }
        else
        {
            siRet = OAM_LinkHsSend();
        }
        break;

    case TM_TYPE_SYNC:
        if(XSUCC == OAM_LinkGet(&stOamLink))
        {
            for (uiLoop = 0; uiLoop < g_AsyncTidMidCnt; uiLoop++)
            {
                /*发送配置请求到agent获取表ID对应的数据记录*/
                if(XSUCC != OAM_AgtSyncReqSend(OAM_MSGTYPE_SYNC, 
                    g_AsyncTidMidArray[uiLoop].uiModuleId, g_AsyncTidMidArray[uiLoop].uiTableId))
                {
                    XOS_PRINT(MD(FID_OAM, PL_ERR), 
                        "OAM_AppRegMsgProc: OAM_AgtReqSend(OAM_MSGTYPE_SYNC) failed\r\n");
                    return XERROR;
                }

                XOS_PRINT(MD(FID_OAM, PL_LOG), 
                    "OAM_LinkTimeOutProc: OAM Send(OAM_MSGTYPE_SYNC)[mid=%d,tid=%d] succ\r\n",
                    g_AsyncTidMidArray[uiLoop].uiModuleId, g_AsyncTidMidArray[uiLoop].uiTableId);
                
                XOS_Sleep(80);
            }

            g_AsyncTidMidCnt=0;
            XOS_MemSet(g_AsyncTidMidArray,0,sizeof(g_AsyncTidMidArray));
            
        }
        break;
        
    default:
        XOS_PRINT(MD(FID_OAM, PL_ERR), "Unknown time out msg");
        break;
    }

    return siRet;
}

/*****************************************************************************
*函数名:OAM_LinkTimerStart
*功能  :重新封装启动定时器函数
*输入  :
*           PTIMER* tHandle
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32  OAM_LinkTimerStart(PTIMER* ptHandle, XU32 len, e_TIMERTYPE mode, 
                         XU32 backpara1, XU32 backpara2, XU32 backpara3, XU32 backpara4)
{
    t_PARA tPara = { 0 };
    t_BACKPARA tBackPara = { 0 };

    if(XNULL == ptHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkTimerStart: ptHandle is NULL\r\n");
        return XERROR;
    }

    tPara.fid = FID_OAM;
    tPara.len = len;
    tPara.mode = mode;
    tPara.pre = TIMER_PRE_LOW;

    tBackPara.para1 = backpara1;
    tBackPara.para2 = backpara2;
    tBackPara.para3 = backpara3;
    tBackPara.para4 = backpara4;

    return XOS_TimerStart(ptHandle, &tPara, &tBackPara);
}

/*****************************************************************************
*函数名:OAM_LinkTimerStop
*功能  :重新封装停止定时器函数
*输入  :
*           PTIMER* tHandle
*输出  :
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkTimerStop(PTIMER* ptHandle)
{
    XS32 siRet = XSUCC;

    if(XNULL == ptHandle)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "OAM_LinkTimerStop: ptHandle is NULL\r\n");
        return XERROR;
    }

    if(*ptHandle)
    {
        siRet = XOS_TimerStop(FID_OAM, *ptHandle);
        *ptHandle = 0;
    }

    return siRet;
}

/*****************************************************************************
*函数名:OAM_LinkGet
*功能  :获取链路
*输入  :
*输出  :
*           OAM_LINK_T* pOamLink
*返回  :
*           XSUCC  成功
*           XERROR 失败
*说明  :
*****************************************************************************/
XS32 OAM_LinkGet(OAM_LINK_T* pOamLink)
{
#ifndef LINUX
    return XSUCC;
#endif

    if(XNULL == pOamLink)
    {
        XOS_PRINT(MD(FID_OAM, PL_ERR), "pOamLink is null\r\n");
        return XERROR;
    }

    if(LINK_STATE_CONNECT == g_OamLinkCB.uiState)
    {
        pOamLink->hLinkHandle = g_OamLinkCB.hLinkHandle;
        pOamLink->stLocalAddr.ip = g_OamLinkCB.uiIpAddr[0];
        pOamLink->stLocalAddr.port = g_OamLinkCB.usPort;        
        return XSUCC;
    }

    OAM_LinkTimerStop(&g_AsyncTmHandle);
    OAM_LinkTimerStart(&g_AsyncTmHandle, 
        LINK_SYNC_TIMEOUT_LEN, TIMER_TYPE_ONCE, TM_TYPE_SYNC, 0, 0, 0);
    
    XOS_PRINT(MD(FID_OAM, PL_DBG), "OAM_LinkGet: valid link NOT FOUND\r\n");
    return XERROR;
}

/*****************************************************************************
*函数名:OAM_LinkAgtBaseIpAddrGet
*功能  :获取agent的两个BASE ip
*输入  :
*           XU32 uiIP agent的一个BASE ip
*输出  :
*           XU32 *puiBase1Ip
*           XU32 *puiBase2Ip

*返回  :
*说明  :
*****************************************************************************/
XVOID OAM_LinkAgtBaseIpAddrGet(XU32 uiIp, XU32 *puiBase1Ip, XU32 *puiBase2Ip)
{
    XS8  ucIpStr[MAX_LINK_NUM][MAX_IPSTR_LEN] = {{0},{0}};
    XS8* pIpstr = XNULL;
    XU32 i      = 0;
    XU32 tmpVal = 0;
    XS8  ucNum[4] = {0, 0, 0, 0};
    //XU32 uiBaseId = 0;    
    XU32 uiSlotId = 0;
    XU32 uiFrameId = 0;

    XOS_IptoStr(uiIp, ucIpStr[0]);

    pIpstr = ucIpStr[0];

    i = 0;
    //find two '.'
    while(1)
    {
        if(*pIpstr == '.')
            i++;

        if(i == 2)
            break;

        pIpstr++;
    }

    i = 0;
    pIpstr++;
    //find frameId, baseId
    while(*pIpstr != '.')
    {
        ucNum[i++] = *pIpstr;
        pIpstr++;
    }

    tmpVal = atoi(ucNum);
    //uiBaseId  = tmpVal % 10;
    uiFrameId = (tmpVal / 10) + 1;

    i = 0;
    XOS_MemSet(ucNum, 0, sizeof(ucNum));
    pIpstr++;
    //find slotId
    while(*pIpstr)
    {
        ucNum[i++] = *pIpstr;
        pIpstr++;
    }

    uiSlotId = atoi(ucNum);

    XOS_MemSet(ucIpStr, 0, sizeof(ucIpStr));
    for(i=0; i<MAX_LINK_NUM; i++)
    {
        /*框号从1开始, ip从0开始:siFrameId - 1*/
        if(uiFrameId > 1)
        {
            XOS_Sprintf(ucIpStr[i], MAX_IPSTR_LEN, "172.16.%d%d.%d", uiFrameId - 1, i, uiSlotId);
        }
        else/*uiFrameId == 1*/
        {
            XOS_Sprintf(ucIpStr[i], MAX_IPSTR_LEN, "172.16.%d.%d", i, uiSlotId);
        }
    }

    XOS_StrtoIp(ucIpStr[0], puiBase1Ip);
    XOS_StrtoIp(ucIpStr[1], puiBase2Ip);    
    
    return;
}

#ifdef __cplusplus
}
#endif
