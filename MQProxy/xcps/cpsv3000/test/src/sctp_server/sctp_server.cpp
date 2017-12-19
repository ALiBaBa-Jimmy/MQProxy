
#include "sctp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 SctpSerTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    XS8 SctpSerInit( XVOID*, XVOID*);
    XS8 SctpSerClose(XVOID *Para1, XVOID *Para2);
    XS8 SctpSerMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 SctpSerTimerOut(t_BACKPARA* para);

    void SctpSerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    void SctpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    void StartSctpSer(XS8* ip, XS32 port);
    void SctpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 stream, XS32 index);
    void SctpSendBig(XS8* peerip, XS32 peerport, XS32 index);
    void SctpClose(XS32 index,XS32 type);
    XS32 SctpLinkRelease(XS32 index);

    
    XS32 SctpLinkInit(t_LINKINIT & linkInit);
    XS32 SctpLinkStart(t_LINKSTART& linkStart);
    XS32 SctpLinkClose(t_LINKCLOSEREQ& linkClose);
    XS32 SctpSendData(XS8* data, XS32 len);
    

t_XOSFIDLIST g_dispatcherSctpServerFid ={
    { "FID_SCTP_SERVER",  XNULL, FID_SCTP_SERVER,},
    { SctpSerInit, NULL, SctpSerClose,},
    { SctpSerMsgProc, SctpSerTimerOut,}, eXOSMode, NULL
};

typedef enum 
{
    SCTP_MSG_SEND_TIMER = 1,
}e_sctpSendTimer;

typedef struct NtlSctpObj {
    XS8    _used;    
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_SCTPSTARTACK   _StartAck;
    struct t_CONNIND_EXT
    {
        t_CONNIND _Conn;
        XU8 used;
    }Conn[100];

    XCHAR localIp[64];
    XU8 localIpNum;
    XU16 localPort;
    XU16 allownClients;
    XU16 curclients;
    XU16 streamNum;
    XU16 hbInterval;
}NtlSctpObj;

typedef struct sendMsg{
    XCHAR ip[64];
    XU16 port;
    XU16 stream;
    XU16 index;
}sendMsg;

#define MaxObj 1200
XSTATIC XS32 g_appNum = 0x01;
XSTATIC XS32 g_index = 0;
XSTATIC NtlSctpObj g_sctpArray[MaxObj];
t_XOSMUTEXID g_sctpArray_Mutex;
XSTATIC PTIMER sctpTestTimer;
sendMsg g_Sendmsg;
#define MAX_SEND_MSG (2000)
XCHAR g_Buf[MAX_SEND_MSG];

XSTATIC XS32 msgNum = 0;
XSTATIC XS32 recvNum = 0;
XSTATIC XS64 recvall = 0;
XSTATIC XS32 recvIndex = 0;
XSTATIC XS32 recvCount = 0;


// ----------------------------- MyTest -----------------------------
XS32 SctpSerTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherSctpServerFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_SCTP_SERVER", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_SCTP_SERVER;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "sctpserver", "sctpserver", "");

    XOS_RegistCommand(promptID, SctpSerCmd, "sctpserver", "start sctp server", "sctpserver ip port number\n"\
                                                                               "sample:\n"\
                                                                               "sctpserver 169.0.199.27 12345");
    XOS_RegistCommand(promptID, SctpSerCmd, "sctpsend", "send message to peer", 
                   "sctpsend peerip peerport sendlen sendnum sid index interval");
    XOS_RegistCommand(promptID, SctpSerCmd, "sctpclose", "close sctp client or server", "sctpclose index type");
    XOS_RegistCommand(promptID, SctpSerCmd, "sctprelease", "release a link", "sctprelease index");
    XOS_RegistCommand(promptID, SctpSerShow, "sctpshow", "show local network info", "sctpshow");

    return XSUCC;
}

XS8 SctpSerInit( XVOID*, XVOID*)
{
    MMInfo("SctpSerInit begin");

    if (XSUCC != XOS_TimerReg(FID_SCTP_SERVER, 200, 1000, 10))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "SctpSerInit()-> register timer error!\n");
        return XERROR;
    }

    XOS_MutexCreate(&g_sctpArray_Mutex);

    MMInfo("SctpSerInit end");
    return XSUCC;
}

XS8 SctpSerClose(XVOID *Para1, XVOID *Para2)
{
    XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SctpSerClose ...");

    XOS_MutexDelete(&g_sctpArray_Mutex);

    return 0;
}

XS8 SctpSerMsgProc( XVOID* inMsg, XVOID*)
{
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
        case eSctpInitAck:
        {
            XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->Get a sctp initAck from NTL");
            t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
            XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "LinkInitAck result=%d", pInitAck->lnitAckResult);
            if(pInitAck->lnitAckResult == eSUCC) 
            {
                XOS_MutexLock(&g_sctpArray_Mutex);
                for(XS32 i = 0; i < MaxObj; i++) 
                {
                    if(1 == g_sctpArray[i]._used && g_sctpArray[i]._app == pInitAck->appHandle) 
                    {
                        XOS_MemCpy(&(g_sctpArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                        /*start*/
                        SctpLinkStartByIndex(i);
                        break;
                    }
                }
                XOS_MutexUnlock(&g_sctpArray_Mutex);
            }
            else
            {
                XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_ERR), "LinkInitAck valid");
            }
        }
        break;
        case eSctpStartAck:
        {
            t_SCTPSTARTACK * pStartAck = (t_SCTPSTARTACK*)pMsg->message;
            XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->Get a sctp startAck res:%d,ip:%x,port:%d",
            pStartAck->linkStartResult,pStartAck->localAddr.ip[0],pStartAck->localAddr.port);
            if(pStartAck->linkStartResult == eSUCC) 
            {
                XOS_MutexLock(&g_sctpArray_Mutex);
                for(XS32 i = 0; i < MaxObj; i++) 
                {
                    if(g_sctpArray[i]._app == pStartAck->appHandle) 
                    {
                        XOS_MemCpy(&(g_sctpArray[i]._StartAck), pStartAck, sizeof(t_SCTPSTARTACK));
                        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "Link started successfully.");
                        break;
                    }
                }
                XOS_MutexUnlock(&g_sctpArray_Mutex);
            }
        }
        break;
    case eSctpConnInd:
    {
        XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->Get a sctp connInd from NTL");
        char ipval[255] = {0};

        t_CONNIND * pConn = (t_CONNIND*)pMsg->message;

        XOS_MutexLock(&g_sctpArray_Mutex);
        for(XS32 i = 0; i < MaxObj; i++) 
        {
            if(g_sctpArray[i]._app == pConn->appHandle) 
            {
                XOS_MemCpy(&(g_sctpArray[i].Conn[g_sctpArray[i].curclients]._Conn), pConn, sizeof(t_CONNIND));
                g_sctpArray[i].Conn[g_sctpArray[i].curclients].used = 1;
                g_sctpArray[i].curclients++;
                XOS_IptoStr(pConn->peerAddr.ip, ipval);
                XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO),
                "Accept a client,ip=%s,port=%d",
                ipval, pConn->peerAddr.port);
                break;
            }
        }
        XOS_MutexUnlock(&g_sctpArray_Mutex);
    }
    break;

    case eSctpDataInd:
    {
        t_SCTPDATAIND *pData = (t_SCTPDATAIND*)pMsg->message;

        msgNum++;
        recvNum += pData->dataLenth;
        recvall += pData->dataLenth;
        XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->recvCount = %d,totalLen:%ld", msgNum,recvall);
        if(pData->pData != XNULL)
        {
            XOS_MemFree(FID_SCTP_SERVER, pData->pData );
        }
    }
    break;

    case eSctpErrorSend:
    {
        t_SENDSCTPERROR *senderror = (t_SENDSCTPERROR *)pMsg->message;
        XOS_Trace(MD(FID_SCTP_SERVER,PL_ERR),"SctpSerMsgProc()->send sctp data error:%d,ip:%x,port:%d",
            senderror->errorReson,
            senderror->peerIp.ip,
            senderror->peerIp.port);
        break;
    }
    default:
        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "get other msgid=%d", pMsg->msgID);
        break;
    }

    //XOS_MemFree(FID_SCTP_SERVER, pMsg);
    return 0;
}


XS8 SctpSerTimerOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SctpSerTimerOut -> %d", para->para1);

    XS32 nIndex = 0;
    if(NULL != para)
    {
        nIndex = para->para1;
        if(nIndex == SCTP_MSG_SEND_TIMER)
        {
            SctpSend(g_Sendmsg.ip, g_Sendmsg.port, g_Buf, g_Sendmsg.stream, g_Sendmsg.index);
        }
    }

    return 0;
}


void SctpStartSendTimer(void)
{
    t_PARA timerpara;
    t_BACKPARA backpara = {0};
    timerpara.fid = FID_SCTP_SERVER;
    timerpara.len = 1000;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_HIGH;
    backpara.para1 = SCTP_MSG_SEND_TIMER;
    
    if (XSUCC != XOS_TimerStart(&sctpTestTimer, &timerpara, &backpara))
    {
        XOS_Trace(MD(FID_SCTP_SERVER, PL_EXP), "ftp start test timer error!");
        return ;
    }
}


void SctpGetSendLenVar(XCHAR* pStr , XU32 *pOut)
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


void SctpSerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    XU16 i = 0;
    XU32 nSendLen = 0;
    if(siArgc < 2) {
    return;
    }
    
    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "command:%s,args:%d.",cmd,siArgc);
    if(!XOS_StrCmp(cmd, "sctpserver")) 
    {
        if(siArgc >= 4) 
        {
            for(int i = 0;i < atoi(ppArgv[3]);i++)
            {
                StartSctpSer(ppArgv[1], atoi(ppArgv[2]) + i);
                XOS_Sleep(200);
            }
        }
    } 
    else if(!XOS_StrCmp(cmd, "sctpsend")) 
    {
        if(siArgc >= 8) 
        {
            SctpGetSendLenVar(ppArgv[3], &nSendLen);
            for(i = 0 ;i < nSendLen; i++)
            {
                g_Buf[i] = 'f';
            }
            g_Buf[i] = '\0';
            //memcpy(g_Sendmsg.ip, ppArgv[1], strlen(ppArgv[1]));
            //g_Sendmsg.port = atoi(ppArgv[2]);
            //g_Sendmsg.stream = atoi(ppArgv[4]);
            //g_Sendmsg.index = atoi(ppArgv[5]);

            //SctpStartSendTimer();

            //SctpSendBig(ppArgv[1], atoi(ppArgv[2]),atoi(ppArgv[5]));

            for(int i = 0;i < atoi(ppArgv[4]); i++)
//            while(1)
            {             
                SctpSend(ppArgv[1], atoi(ppArgv[2]), g_Buf, atoi(ppArgv[5]), 
                                                            atoi(ppArgv[6]));
                XOS_Sleep(atoi(ppArgv[7]));
            }
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "sctpsend complete");
        }
        else
        {
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR:sctpsend args error.");
        }
    }
    else if(!XOS_StrCmp(cmd, "sctpclose")) 
    {
        if(siArgc >= 2) 
        {
            SctpClose(atoi(ppArgv[1]),atoi(ppArgv[2]));
            msgNum = 1;
        }
        else
        {
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR:SctpClose args error.");
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

void SctpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 index = -1;
    XS32 i =0 ;
    XS32 j = 0;

    /*tcp client cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "sctp server statistic list \r\n----------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-6s%-12s\r\n",
        "index",
        "serveraddr",
        "listenport",
        "type"
        );

    XOS_MutexLock(&g_sctpArray_Mutex);
    for(i=0; i<MaxObj; i++)
    {
        if(g_sctpArray[i]._used)
        {
            XOS_CliExtPrintf(pCliEnv,
                "----------------------------\r\n");
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-6d   sctp\r\n",
                i,g_sctpArray[i]._StartAck.localAddr.ip[0],g_sctpArray[i]._StartAck.localAddr.port
                );
            if(g_sctpArray[i].curclients >0)
            {            
                for(j = 0; j<g_sctpArray[i].curclients; j++ )
                {
                    
                    XOS_CliExtPrintf(pCliEnv,
                        "%-6d%-12x%-6d    client\r\n",
                        j,g_sctpArray[i].Conn[j]._Conn.peerAddr.ip,g_sctpArray[i].Conn[j]._Conn.peerAddr.port
                        );
                }
            }

        }
    }
    XOS_MutexUnlock(&g_sctpArray_Mutex);

    XOS_CliExtPrintf(pCliEnv,
        "recvall = %lld, msgNum = %d, recvNum = %d", recvall, msgNum, recvNum );

    recvall = 0;

    /*end of list */
    XOS_CliExtPrintf(pCliEnv,
        "----------------------------\r\n");
    return ;
}


void SctpLinkStartByIndex(XU16 index)
{
    t_LINKSTART linkStart;
    
    linkStart.linkHandle = g_sctpArray[index]._IniAck.linkHandle;
    XOS_StrtoIp(g_sctpArray[index].localIp, &(linkStart.linkStart.sctpServerStart.myAddr.ip[0]));
    linkStart.linkStart.sctpServerStart.myAddr.ipNum = g_sctpArray[index].localIpNum;
    linkStart.linkStart.sctpServerStart.myAddr.port = g_sctpArray[index].localPort;
    linkStart.linkStart.sctpServerStart.allownClients = g_sctpArray[index].allownClients;
    linkStart.linkStart.sctpServerStart.streamNum = g_sctpArray[index].streamNum;
    linkStart.linkStart.sctpServerStart.hbInterval = g_sctpArray[index].hbInterval;
    linkStart.linkStart.sctpServerStart.authenFunc = NULL;
    if(XSUCC != SctpLinkStart(linkStart)) 
    {
        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SctpLinkStart failed,port=%d", g_sctpArray[index].localPort);
    }    
}

void StartSctpSer(XS8* ip, XS32 port)
{
    t_LINKINIT linkInit;
    XS32 i = 0;

    linkInit.linkType = eSCTPServer;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;
    XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "StartSctpSer, ip=%s,port=%d", ip,port);

    /*获取一个可用的控制块*/
    XOS_MutexLock(&g_sctpArray_Mutex);
    for(i = 0; i < MaxObj; i++)
    {
        if(!g_sctpArray[i]._used) 
        {
            g_sctpArray[i]._app = linkInit.appHandle;
            g_sctpArray[i]._used = 1;
            memcpy(g_sctpArray[i].localIp, ip, strlen(ip));
            g_sctpArray[i].localIpNum = 1;
            g_sctpArray[i].localPort = port;
            g_sctpArray[i].allownClients = 2000;
            g_sctpArray[i].curclients = 0;
            g_sctpArray[i].streamNum = 3;
            g_sctpArray[i].hbInterval = 20000; 
            memset(g_sctpArray[i].Conn, 0, sizeof(g_sctpArray[i].Conn));
            break;
        }
    }
    XOS_MutexUnlock(&g_sctpArray_Mutex);
    
    if(i == MaxObj) 
    {
        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "Test Link is enough, max=%d", MaxObj);
        return;
    }
    
    if(XSUCC != SctpLinkInit(linkInit)) 
    { 
         XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "LinkInit send fail");
    } 
    
}

void SctpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 stream, XS32 index)
{
    t_SCTPDATAREQ dataReq;
    XS32 i = 0;

    XS8* pMsg = (XS8*)XOS_MemMalloc(FID_SCTP_SERVER, strlen(msg));
    XOS_MemSet(pMsg, 0, strlen(msg));

    XOS_StrtoIp(peerip,&(dataReq.dstAddr.ip));
    dataReq.dstAddr.port = peerport;
    dataReq.linkHandle = g_sctpArray[index]._IniAck.linkHandle;
    dataReq.msgLenth = strlen(msg);
    dataReq.attr.stream = stream;
    dataReq.attr.ppid = 0;
    dataReq.attr.context = 0;
    dataReq.pData = pMsg;

    XOS_MemCpy(dataReq.pData, msg, strlen(msg));
    XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SendData ip:%x,port:%d,len:%d,stream:%d\n",
    dataReq.dstAddr.ip,dataReq.dstAddr.port,strlen(msg),stream);

    SctpSendData((XS8 *)&dataReq, sizeof(t_SCTPDATAREQ));
}

void SctpSendBig(XS8* peerip, XS32 peerport, XS32 index)
{
    t_SCTPDATAREQ dataReq;
    XS32 i = 0;
    XS32 msgLen = 50;
    XS8* pMsg = (XS8*)XOS_MemMalloc(FID_SCTP_SERVER, msgLen + 1);

    XOS_MemSet(pMsg, 0, msgLen + 1);
    for(i = 0;i< msgLen;i++)
    {
        pMsg[i] = 'a';
    }
    XOS_StrtoIp(peerip,&(dataReq.dstAddr.ip));
    dataReq.dstAddr.port = peerport;
    dataReq.linkHandle = g_sctpArray[index]._IniAck.linkHandle;
    dataReq.msgLenth = msgLen;
    dataReq.attr.stream = 0;
    dataReq.attr.ppid = 0;
    dataReq.attr.context = 0;
    dataReq.pData = pMsg;

    XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SendData ip:%x,port:%d ,a big message",dataReq.dstAddr.ip,dataReq.dstAddr.port);

    SctpSendData((XS8 *)&dataReq, sizeof(t_SCTPDATAREQ));
}

void SctpClose(XS32 index,XS32 type)
{
    if(index < 0 || index >= MaxObj )
    {
        return;
    }
    if(g_sctpArray[index]._used == 0)
    {
        return;
    }
    t_LINKCLOSEREQ *closeReq = NULL;
    closeReq = (t_LINKCLOSEREQ *)XOS_MemMalloc(FID_SCTP_SERVER, sizeof(t_LINKCLOSEREQ));
    XOS_MemSet(closeReq, 0, sizeof(t_LINKCLOSEREQ));

    closeReq->linkHandle = g_sctpArray[index]._IniAck.linkHandle;
    if(type == 0)
    {
        closeReq->cliAddr.ip = g_sctpArray[index].Conn[0]._Conn.peerAddr.ip;
        closeReq->cliAddr.port = g_sctpArray[index].Conn[0]._Conn.peerAddr.port;

        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SctpClose ip:%x,port:%d",
            g_sctpArray[index].Conn[0]._Conn.peerAddr.ip,g_sctpArray[index].Conn[0]._Conn.peerAddr.port);
    }
    else
    {
        closeReq->cliAddr.ip = 0;
        closeReq->cliAddr.port = 0;

        XOS_CpsTrace(MD(FID_SCTP_SERVER, PL_INFO), "SctpClose ip:%s,port:%d",
            g_sctpArray[index].localIp,g_sctpArray[index].localPort);
    }

    if(type == 1)
    {
        g_sctpArray[index]._used = 0;
    }   

    SctpLinkClose(*closeReq);
}

XS32 SctpLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_SERVER, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_SERVER, pMsg);
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
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

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_SERVER, sizeof(t_LINKSTART));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_SERVER, pMsg);
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR: TcpSvrLinkInit send msg failed.");
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

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_SERVER, sizeof(t_LINKCLOSEREQ));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStop;
        XOS_MemCpy(pMsg->message, &linkClose, sizeof(t_LINKCLOSEREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_SERVER, pMsg);
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR: LinkClose send msg failed.");
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

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_SERVER, len);
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_SERVER, pMsg);
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR: SctpSendData send data failed.");
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

    linkRelease.linkHandle = g_sctpArray[index]._IniAck.linkHandle;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_SCTP_SERVER, sizeof(t_LINKRELEASE));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_SCTP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkRelease;
        XOS_MemCpy(pMsg->message, &linkRelease, sizeof(t_LINKRELEASE));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_SCTP_SERVER, pMsg);
            XOS_Trace(FILI, FID_SCTP_SERVER, PL_ERR, "ERROR: SctpLinkRelease failed.");
            return XERROR;
        }
        g_sctpArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

