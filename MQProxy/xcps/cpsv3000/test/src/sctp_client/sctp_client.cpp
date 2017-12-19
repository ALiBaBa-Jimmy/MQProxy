
#include "sctp_client.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 SctpCliTest(HANDLE hdir, XS32 argc, XCHAR** argv);

    XS8 SctpCliInit( XVOID*, XVOID*);
    XS8 SctpCliClose(XVOID *Para1, XVOID *Para2);
    XS8 SctpCliMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 SctpCliMsgOut(t_BACKPARA* para);

    void SctpCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    void SctpCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

XSTATIC void StartSctpCli(XS8* ip, XS32 port, XS8* peerip, XS32 peerport);
XSTATIC void SctpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 stream, XS32 count);
XSTATIC    void SctpClose(XS32 index);
XSTATIC    XS32 SctpLinkRelease(XS32 index);

XSTATIC XS32 SctpLinkInit(t_LINKINIT & linkInit);
XSTATIC XS32 SctpLinkStart(t_LINKSTART& linkStart);
XSTATIC    XS32 SctpLinkClose(t_LINKCLOSEREQ& linkClose);
XSTATIC XS32 SctpSendData(XS8* data, XS32 len);
    

t_XOSFIDLIST g_dispatcherSctpClientFid ={
    { "FID_SCTP_CLIENT",  XNULL, FID_SCTP_CLIENT,},
    { SctpCliInit, NULL, SctpCliClose,},
    { SctpCliMsgProc, SctpCliMsgOut,}, eXOSMode, NULL
};

typedef struct NtlSctpCliObj {
    XS8    _used;
    t_XOSSEMID _Semid;
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_SCTPSTARTACK   _StartAck;
    t_CONNIND _Conn;
}NtlSctpCliObj;

#define MaxObj 600
XSTATIC XS32 g_appNum = 0x01;
XSTATIC XS32 g_index = 0;
XSTATIC NtlSctpCliObj g_sctpCliArray[MaxObj];
XSTATIC XS32 msgNum = 0;
XSTATIC XS64 recvall = 0;
XSTATIC XS64 recvCount =0 ;
XCHAR g_SendBuf[8000];


// ----------------------------- MyTest -----------------------------
XS32 SctpCliTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherSctpClientFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_SCTP_CLIENT", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_SCTP_CLIENT;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "sctpclient", "sctpclient", "");

    XOS_RegistCommand(promptID, SctpCliCmd, "sctpclient", "start sctp client", 
                      "sctpclient localip localport peerip peerport number");
    XOS_RegistCommand(promptID, SctpCliCmd, "sctpsend", "send message to peer", 
                 "sctpsend peerip peerport sendlen sendnum sid  index interval");
    XOS_RegistCommand(promptID, SctpCliCmd, "sctpclose", "close sctp client", "sctpclose index");
    XOS_RegistCommand(promptID, SctpCliCmd, "sctprelease", "release a link", "sctprelease index");
    XOS_RegistCommand(promptID, SctpCliShow, "sctpshow", "show local network info", "sctpshow");

    return XSUCC;
}

XS8 SctpCliInit( XVOID*, XVOID*)
{
    MMInfo("SctpCliInit begin");

    for(XS32 i = 0; i < MaxObj; i++) {
        XOS_SemCreate(&(g_sctpCliArray[i]._Semid), 0);
        g_sctpCliArray[i]._used = 0;
    }    

    MMInfo("SctpCliInit end");
    return XSUCC;
}

XS8 SctpCliClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "SctpCliClose ...");

    for(XS32 i = 0; i < MaxObj; i++) {
        XOS_SemDelete(&(g_sctpCliArray[i]._Semid));
    }

    return 0;
}

XS8 SctpCliMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"SctpCliMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
        case eSctpInitAck:
        {
            t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
            XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "LinkInitAck result=%d", pInitAck->lnitAckResult);
            if(pInitAck->lnitAckResult == eSUCC) 
            {
                for(XS32 i = 0; i < MaxObj; i++) 
                {
                    if(g_sctpCliArray[i]._app == pInitAck->appHandle) 
                    {
                        XOS_MemCpy(&(g_sctpCliArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                        XOS_SemPut(&(g_sctpCliArray[i]._Semid));
                        break;
                    }
                }
            }
        }
        break;
        case eSctpStartAck:
        {
            t_SCTPSTARTACK * pStartAck = (t_SCTPSTARTACK*)pMsg->message;
            XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"SctpCliMsgProc()->Get a sctp startAck res:%d,ip:%x,port:%d",
            pStartAck->linkStartResult,pStartAck->localAddr.ip[0],pStartAck->localAddr.port);
            if(pStartAck->linkStartResult == eSUCC) 
            {
                for(XS32 i = 0; i < MaxObj; i++) 
                {
                    if(g_sctpCliArray[i]._app == pStartAck->appHandle) 
                    {
                        XOS_MemCpy(&(g_sctpCliArray[i]._StartAck), pStartAck, sizeof(t_SCTPSTARTACK));
                        g_index = i;
                        XOS_SemPut(&(g_sctpCliArray[i]._Semid));
                        XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "Link started successfully.");
                        break;
                    }
                }
            }
        }
        break;
       case eSctpConnInd:
        {
            XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"SctpCliMsgProc()->Get a sctp connInd from NTL");
            char ipval[255] = {0};

            t_CONNIND * pConn = (t_CONNIND*)pMsg->message;
            for(XS32 i = 0; i < MaxObj; i++) 
            {
                if(g_sctpCliArray[i]._app == pConn->appHandle) 
                {
                    XOS_MemCpy(&(g_sctpCliArray[i]._Conn), pConn, sizeof(t_CONNIND));
                    XOS_IptoStr(pConn->peerAddr.ip, ipval);
                    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO),
                    "Accept a client,ip=%s,port=%d",
                    ipval, pConn->peerAddr.port);
                    break;
                }
            }
        }
        break;

        case eSctpDataInd:
        {
            XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"SctpCliMsgProc()->Get a sctp dataInd from NTL recvCount=%d", ++recvCount);
            char ipval[255] = {0};
            t_SCTPDATAIND *pData = (t_SCTPDATAIND*)pMsg->message;
            msgNum++;
            recvall += pData->dataLenth;

            if(pData) 
            {
                XOS_IptoStr(pData->peerAddr.ip, ipval);
                if(pData->dataLenth > 0 && pData->pData) 
                {

                    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO),
                    "Recv Msg,[%d]app:%d,ip=%s,port=%d,stream=%d,ppid=%d,context=%d ,totallen:%ld",
                    msgNum,
                    pData->appHandle,
                    ipval,
                    pData->peerAddr.port,
                    pData->attr.stream,
                    pData->attr.ppid,
                    pData->attr.context,
                    recvall
                    );                    
                }
                if(pData->pData != XNULL)
                {
                    XOS_MemFree(FID_SCTP_CLIENT, pData->pData );
                }
            }
        }
        break;
        case eSctpErrorSend:
        {
            t_SENDSCTPERROR *senderror = (t_SENDSCTPERROR *)pMsg->message;
            XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"SctpCliMsgProc()->send sctp data error:%d,ip:%x,port:%d",
                senderror->errorReson,
                senderror->peerIp.ip,
                senderror->peerIp.port
                );
            break;
        }
       default:
        XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "get other msgid=%d", pMsg->msgID);
        break;
       }

    return 0;
}

XS8 SctpCliMsgOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "SctpCliMsgOut -> %d", para->para1);

    return 0;
}

void SctpGetSendLen(XCHAR* pStr , XU32 *pOut)
{
    XU32  nSendLen = 0;

    if(NULL == pStr)
    {
        *pOut = 1452;  
        return;
    }
    
    nSendLen = atoi(pStr);
    if(nSendLen >= 8000)
    {
        *pOut = 8000;
    }
    else
    {
        *pOut = nSendLen;
    }  
    
}


void SctpCliCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    XS32 i = 0;
    XU32 nSendLen = 0;
    if(siArgc < 2) {
        return;
    }

    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
//    XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "command:%s,args:%d.",cmd,siArgc);
    if(!XOS_StrCmp(cmd, "sctpclient")) 
    {
        if(siArgc >= 6) 
        {
            int i = 0;
            int j = 0;
            for(i = 0;i < atoi(ppArgv[5]);i++)
            {
                StartSctpCli(ppArgv[1], atoi(ppArgv[2]) + j, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "client:%s,%d,peer:%s,%d.",
                ppArgv[1], atoi(ppArgv[2]) + j, ppArgv[3], atoi(ppArgv[4]) + i);
            
              XOS_Sleep(100);
                /*StartSctpCli(ppArgv[1], atoi(ppArgv[2]) + j + 1, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "client:%s,%d,peer:%s,%d.",
                ppArgv[1], atoi(ppArgv[2]) + j + 1, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Sleep(100);
                
                StartSctpCli(ppArgv[1], atoi(ppArgv[2]) + j + 2, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "client:%s,%d,peer:%s,%d.",
                ppArgv[1], atoi(ppArgv[2]) + j + 2, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Sleep(100);
                StartSctpCli(ppArgv[1], atoi(ppArgv[2]) + j + 3, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "client:%s,%d,peer:%s,%d.",
                ppArgv[1], atoi(ppArgv[2]) + j + 3, ppArgv[3], atoi(ppArgv[4]) + i);
                XOS_Sleep(100);*/
                j += 2;
                
            }
        }
    } 
    else if(!XOS_StrCmp(cmd, "sctpsend")) 
    {
        if(siArgc >= 8) 
        {
            SctpGetSendLen(ppArgv[3], &nSendLen);
            for(i = 0; i < nSendLen; i++)
            {
                g_SendBuf[i] = 'a';
            }
            
            for(i = 0;i < atoi(ppArgv[4]);i++)
//            while(1)
            {
                SctpSend(ppArgv[1], atoi(ppArgv[2]), g_SendBuf, atoi(ppArgv[5]), 
                                                                atoi(ppArgv[6]));
                XOS_Sleep(atoi(ppArgv[7]));
            }

            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "sctpsend complete");
        }
        else
        {
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR:sctpsend args error.");
        }
    }
    else if(!XOS_StrCmp(cmd, "sctpclose")) 
    {
        if(siArgc >= 2) 
        {
            SctpClose(atoi(ppArgv[1]));
            msgNum = 1;
        }
        else
        {
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR:SctpClose args error.");
        }
    }
    else if(!XOS_StrCmp(cmd, "sctprelease")) 
    {
        if(siArgc >= 2) 
        {
            SctpLinkRelease(atoi(ppArgv[1]));
        }
    }

}

void SctpCliShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 index = -1;
    XS32 i =0 ;
    XS32 j = 0;

    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "tcp client statistic list \r\n----------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-6s%-12s\r\n",
        "index",
        "IP",
        "port",
        "type"
        );

    for(i=0; i<MaxObj; i++)
    {
        if(g_sctpCliArray[i]._used)
        {
            XOS_CliExtPrintf(pCliEnv,
            "%-6d%-12x%-6d   sctp\r\n",
            i,g_sctpCliArray[i]._StartAck.localAddr.ip[0],g_sctpCliArray[i]._StartAck.localAddr.port
            );
        }
    }

    /*end of list */
    XOS_CliExtPrintf(pCliEnv,
        "----------------------------\r\n");

    XOS_CliExtPrintf(pCliEnv,
            "recvall = %lld", recvall);

    return ;
}

void StartSctpCli(XS8* ip, XS32 port, XS8* peerip, XS32 peerport)
{
    t_LINKINIT linkInit;
    t_LINKSTART linkStart;
    XS32 index = 0;
    XS32 i = 0;

    linkInit.linkType = eSCTPClient;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;

    for(i = 0; i < MaxObj; i++) {
        if(!g_sctpCliArray[i]._used) {
            g_sctpCliArray[i]._app = linkInit.appHandle;
            g_sctpCliArray[i]._used = 1;
            index = i;
            break;
        }
    }
    if(i == MaxObj) {
        XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "Test Link is enough, max=%d", MaxObj);
        return;
    }

    if(SctpLinkInit(linkInit) == XSUCC) {
        if(XOS_SemGetExt(&(g_sctpCliArray[index]._Semid), 5) == XSUCC) {
            linkStart.linkHandle = g_sctpCliArray[index]._IniAck.linkHandle;
            XOS_StrtoIp(ip,&(linkStart.linkStart.sctpClientStart.myAddr.ip[0]));
            linkStart.linkStart.sctpClientStart.myAddr.ipNum = 1;
            linkStart.linkStart.sctpClientStart.myAddr.port = port;
            XOS_StrtoIp(peerip,&(linkStart.linkStart.sctpClientStart.peerAddr.ip[0]));
            linkStart.linkStart.sctpClientStart.peerAddr.ipNum = 1;
            linkStart.linkStart.sctpClientStart.peerAddr.port = peerport;
            linkStart.linkStart.sctpClientStart.streamNum = 4;
            linkStart.linkStart.sctpClientStart.hbInterval = 10000;
            XOS_Trace(MD(FID_SCTP_CLIENT,PL_ERR),"StartSctpCli()->ip:%x,port:%d,peerip:%x,port:%d",
            linkStart.linkStart.sctpClientStart.myAddr.ip[0],linkStart.linkStart.sctpClientStart.myAddr.port,
            linkStart.linkStart.sctpClientStart.peerAddr.ip[0],linkStart.linkStart.sctpClientStart.peerAddr.port);

            if(SctpLinkStart(linkStart) == XSUCC) {
                XOS_SemGetExt(&(g_sctpCliArray[index]._Semid), 5);
            } else {
                XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "SctpLinkStart failed,port=%d", port);
            }
        } else {
            XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "LinkInit wait response timeout");
        }
    } else {
        XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "LinkInit send fail");
    }
}

void SctpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 stream, XS32 index)
{
    t_SCTPDATAREQ dataReq;
    XS32 i = 0;

    if(strlen(msg) <= 0 )
    {
        return ;
    }

    XS8* pMsg = (XS8*)XOS_MemMalloc(FID_SCTP_CLIENT, strlen(msg));
    XOS_MemSet(pMsg, 0, strlen(msg));

    XOS_StrtoIp(peerip,&(dataReq.dstAddr.ip));
    dataReq.dstAddr.port = peerport;
    dataReq.linkHandle = g_sctpCliArray[index]._IniAck.linkHandle;
    dataReq.msgLenth = strlen(msg);
    dataReq.attr.stream = stream;
    dataReq.attr.ppid = 0;
    dataReq.attr.context = 0;
    dataReq.pData = pMsg;

    XOS_MemCpy(dataReq.pData, msg, strlen(msg));
    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "SendData ip:%x,port:%d,len:%d,stream:%d",
        dataReq.dstAddr.ip,dataReq.dstAddr.port,strlen(msg),stream);

    SctpSendData((XS8 *)&dataReq, sizeof(t_SCTPDATAREQ));
}

void SctpClose(XS32 index)
{
    if(index < 0 || index >= MaxObj )
    {
        return;
    }
    if(g_sctpCliArray[index]._used == 0)
    {
        return;
    }
    t_LINKCLOSEREQ *closeReq = NULL;
    closeReq = (t_LINKCLOSEREQ *)XOS_MemMalloc(FID_SCTP_CLIENT, sizeof(t_LINKCLOSEREQ));
    XOS_MemSet(closeReq, 0, sizeof(t_LINKCLOSEREQ));

    closeReq->linkHandle = g_sctpCliArray[index]._IniAck.linkHandle;

    closeReq->cliAddr.ip = g_sctpCliArray[index]._StartAck.localAddr.ip[0];
    closeReq->cliAddr.port = g_sctpCliArray[index]._StartAck.localAddr.port;

    g_sctpCliArray[index]._used = 0;
    XOS_CpsTrace(MD(FID_SCTP_CLIENT, PL_INFO), "SctpClose ip:%x,port:%d",
    g_sctpCliArray[index]._StartAck.localAddr.ip[0],g_sctpCliArray[index]._StartAck.localAddr.port);

    SctpLinkClose(*closeReq);
}

XS32 SctpLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_CLIENT, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_CLIENT, pMsg);
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 SctpLinkStart(t_LINKSTART& linkStart)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_CLIENT, sizeof(t_LINKSTART));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_CLIENT, pMsg);
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 SctpLinkClose(t_LINKCLOSEREQ& linkClose)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_CLIENT, sizeof(t_LINKCLOSEREQ));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStop;
        XOS_MemCpy(pMsg->message, &linkClose, sizeof(t_LINKCLOSEREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_CLIENT, pMsg);
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR: LinkClose send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 SctpSendData(XS8* data, XS32 len)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_CLIENT, len);
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_CLIENT, pMsg);
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR: SctpSend send data failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}


XS32 SctpLinkRelease(XS32 index)
{
    t_LINKRELEASE linkRelease;
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    linkRelease.linkHandle = g_sctpCliArray[index]._IniAck.linkHandle;
    g_sctpCliArray[index]._used = 0;
    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_CLIENT, sizeof(t_LINKRELEASE));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_CLIENT;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkRelease;
        XOS_MemCpy(pMsg->message, &linkRelease, sizeof(t_LINKRELEASE));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_CLIENT, pMsg);
            XOS_Trace(FILI, FID_SCTP_CLIENT, PL_ERR, "ERROR: SctpLinkRelease failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

