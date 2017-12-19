
#include "udp_server.h"

#ifdef __cplusplus
extern "C" {
#endif

    XS32 UdpSerTest(HANDLE hdir, XS32 argc, XCHAR** argv);
    XS8 UdpSerInit( XVOID*, XVOID*);
    XS8 UdpSerMsgProc(XVOID *Para1, XVOID *Para2);
    XS8 UdpSerTimerOut(t_BACKPARA* para);

    void UdpSerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);
    void UdpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv);

    void StartUdpSer();
    void UdpSend(XS8* peerip, XS32 peerport, XS8* msg,XS32 len, XS32 index);
    void UdpClose(XS32 index);
    XS32 UdpLinkRelease(XS32 index);
    
    XS32 UdpLinkInit(t_LINKINIT & linkInit);
    XS32 UdpLinkStart(t_LINKSTART& linkStart);
    XS32 UdpLinkClose(t_LINKCLOSEREQ& linkClose);
    XS32 UdpSendData(XS8* data, XS32 len);
    

t_XOSFIDLIST g_dispatcherUdpServerFid ={
    { "FID_UDP_SERVER",  XNULL, FID_UDP_SERVER,},
    { UdpSerInit, NULL, NULL,},
    { UdpSerMsgProc, UdpSerTimerOut,}, eXOSMode, NULL
};

typedef enum 
{
    UDP_MSG_SEND_TIMER = 1,
};

typedef struct NtlUdpObj {
    XS8    _used;    
    HAPPUSER   _app;

    t_LINKINITACK _IniAck;
    t_STARTACK   _StartAck;
    t_CONNIND _Conn[10];

    XU16 curclients;
}NtlUdpObj;

typedef struct sendMsg{
    XCHAR ip[64];
    XU16 port;
    XU16 index;
}sendMsg;

#define MaxObj 1200
XSTATIC XS32 g_appNum = 0x01;
XSTATIC XS32 g_index = 0;
XSTATIC NtlUdpObj g_udpArray[MaxObj];
t_XOSMUTEXID g_udpArray_Mutex;
XSTATIC PTIMER udpTestTimer;
sendMsg g_udpSendMsg = {0};
#define MAX_SEND_MSG (2000)
XCHAR g_udpSendBuf[MAX_SEND_MSG];

XSTATIC XS32 msgNum = 0;
XSTATIC XS32 recvNum = 0;
XSTATIC XS64 recvall = 0;
XSTATIC XS32 recvIndex = 0;
XSTATIC XS32 recvCount = 0;
static XCHAR g_localIp[24];
static XS32 g_localPort = 20000;
static XCHAR g_PeerIp[24];
static XS32 g_peerPort = 40000;


// ----------------------------- MyTest -----------------------------
XS32 UdpSerTest(HANDLE hdir, XS32 argc, XCHAR** argv)
{
    t_XOSLOGINLIST stLoginList;
    XS32 ret = XSUCC;
    XS32 promptID;

    XOS_MemSet(&stLoginList, 0, sizeof(t_XOSLOGINLIST));
    stLoginList.stack = &g_dispatcherUdpServerFid;
    XOS_StrNcpy(stLoginList.taskname, "Tsk_UDP_SERVER", MAX_TID_NAME_LEN);
    stLoginList.TID = FID_UDP_SERVER;
    stLoginList.prio = TSK_PRIO_NORMAL;
    stLoginList.quenum = MAX_MSGS_IN_QUE;
    stLoginList.stacksize = 1024*10;
    ret = XOS_MMStartFid(&stLoginList, XNULLP, XNULLP);

    promptID = XOS_RegistCmdPrompt(SYSTEM_MODE, "test", "test modules", "");

    promptID = XOS_RegistCmdPrompt(promptID, "udpserver", "udpserver", "");

    XOS_RegistCommand(promptID, UdpSerCmd, "udpserver", "start udp server", "udpserver ip port number\n"\
                                                                               "sample:\n"\
                                                                               "udpserver 169.0.199.27 12345");
    XOS_RegistCommand(promptID, UdpSerCmd, "udpsend", "send message to peer", 
                   "udpsend peerip peerport sendlen sendnum index");
    XOS_RegistCommand(promptID, UdpSerCmd, "udpclose", "close udp client or server", "udpclose index");
    XOS_RegistCommand(promptID, UdpSerCmd, "udprelease", "release a link", "udprelease index");
    XOS_RegistCommand(promptID, UdpSerShow, "udpshow", "show local network info", "udpshow");

    return XSUCC;
}

XS8 UdpSerInit( XVOID*, XVOID*)
{
    MMInfo("UdpSerInit begin");

    if (XSUCC != XOS_TimerReg(FID_UDP_SERVER, 200, 1000, 10))
    {
        XOS_Trace(MD(FID_FTP, PL_ERR), "UdpSerInit()-> register timer error!\n");
        return XERROR;
    }

    XOS_MutexCreate(&g_udpArray_Mutex);

    MMInfo("UdpSerInit end");
    return XSUCC;
}

XS8 UdpSerMsgProc( XVOID* inMsg, XVOID*)
{
    t_LINKSTART linkStart;
    int index = 0;
    t_XOSCOMMHEAD *pMsg = (t_XOSCOMMHEAD*)inMsg;
    if( (pMsg == (XVOID*)XNULLP) || pMsg->message == XNULLP)
    {
        XOS_Trace(MD(FID_UDP_SERVER,PL_ERR),"UdpSerMsgProc()->input param error!");
        return XERROR;
    }

    switch (pMsg->msgID) {
        case eInitAck:
        {
            t_LINKINITACK * pInitAck = (t_LINKINITACK*)pMsg->message;
            printf("NtlServerTest LinkInitAck result=%d,apphandle:%d,linkhandle:%x\n", 
                pInitAck->lnitAckResult,pInitAck->appHandle,pInitAck->linkHandle);

            if(pInitAck->lnitAckResult == eSUCC) 
            {
               for(int i = 0; i < MaxObj; i++)
               {
                   if(g_udpArray[i]._app == pInitAck->appHandle)
                   {
                       XOS_MemCpy(&(g_udpArray[i]._IniAck), pInitAck, sizeof(t_LINKINITACK));
                       index = i;
                       break;
                   }
               }
               
               linkStart.linkHandle = g_udpArray[index]._IniAck.linkHandle;
               XOS_StrtoIp(g_localIp,&(linkStart.linkStart.udpStart.myAddr.ip));
                linkStart.linkStart.udpStart.myAddr.port = g_localPort++;
                XOS_StrtoIp(g_PeerIp,&(linkStart.linkStart.udpStart.peerAddr.ip));
                linkStart.linkStart.udpStart.peerAddr.port = g_peerPort++;
                
                XOS_CpsTrace(MD(FID_NTL_SERVER, PL_INFO), "start udp server IP :%x,port:%d.peer ip:%x,port:%d",
                    linkStart.linkStart.udpStart.myAddr.ip,
                    linkStart.linkStart.udpStart.myAddr.port,
                    linkStart.linkStart.udpStart.peerAddr.ip,
                    linkStart.linkStart.udpStart.peerAddr.port);
                if(UdpLinkStart(linkStart) != XSUCC)
                {
                    printf("send UdpSerTest::NtlLinkStart fail!, port=%d\n", g_localPort - 1);
                }
            }
        }
        break;
        case eStartAck:
        {
            t_STARTACK * pStartAck = (t_STARTACK*)pMsg->message;
            XOS_Trace(MD(FID_UDP_SERVER,PL_ERR),"UdpSerMsgProc()->Get a udp startAck res:%d,ip:%x,port:%d",
            pStartAck->linkStartResult,pStartAck->localAddr.ip,pStartAck->localAddr.port);
            if(pStartAck->linkStartResult == eSUCC) 
            {
                XOS_MutexLock(&g_udpArray_Mutex);
                for(XS32 i = 0; i < MaxObj; i++) 
                {
                    if(g_udpArray[i]._app == pStartAck->appHandle) 
                    {
                        XOS_MemCpy(&(g_udpArray[i]._StartAck), pStartAck, sizeof(t_STARTACK));
                        XOS_CpsTrace(MD(FID_UDP_SERVER, PL_INFO), "Link started successfully.");
                        break;
                    }
                }
                XOS_MutexUnlock(&g_udpArray_Mutex);
            }
        }
        break;
    case eConnInd:
    {
        XOS_Trace(MD(FID_UDP_SERVER,PL_ERR),"UdpSerMsgProc()->Get a udp connInd from NTL");
        char ipval[255] = {0};

        t_CONNIND * pConn = (t_CONNIND*)pMsg->message;

        XOS_MutexLock(&g_udpArray_Mutex);
        for(XS32 i = 0; i < MaxObj; i++) 
        {
            if(g_udpArray[i]._app == pConn->appHandle) 
            {
                XOS_MemCpy(&(g_udpArray[i]._Conn[g_udpArray[i].curclients]), pConn, sizeof(t_CONNIND));
                g_udpArray[i].curclients++;
                XOS_IptoStr(pConn->peerAddr.ip, ipval);
                XOS_CpsTrace(MD(FID_UDP_SERVER, PL_ERR),
                "Accept a client,apphandle:0x%x,ip=%s,port=%d",
                pConn->appHandle,ipval, pConn->peerAddr.port);

                //UdpStartSendTimer();
                break;
            }
        }
        XOS_MutexUnlock(&g_udpArray_Mutex);
    }
    break;

    case eDataInd:
    {
        char ipval[255] = {0};
        t_DATAIND *pData = (t_DATAIND*)pMsg->message;

        msgNum++;
        recvNum += pData->dataLenth;
        recvall += pData->dataLenth;
        XOS_Trace(MD(FID_UDP_SERVER,PL_ERR),"UdpSerMsgProc()->ip:%x,port:%d,recvCount = %d,totalLen:%ld",
            pData->peerAddr.ip,pData->peerAddr.port,msgNum,recvall);
        if(pData->pData != XNULL)
        {
            XOS_MemFree(FID_UDP_SERVER, pData->pData );
        }
    }
    break;

    case eErrorSend:
    {
        t_SENDERROR *senderror = (t_SENDERROR *)pMsg->message;
        XOS_Trace(MD(FID_UDP_SERVER,PL_ERR),"UdpSerMsgProc()->send udp data error:%d",
            senderror->errorReson);
        break;
    }
    default:
        XOS_CpsTrace(MD(FID_UDP_SERVER, PL_INFO), "get other msgid=%d", pMsg->msgID);
        break;
    }

    //XOS_MemFree(FID_UDP_SERVER, pMsg);
    return 0;
}


XS8 UdpSerTimerOut(t_BACKPARA* para)
{
    XOS_CpsTrace(MD(FID_UDP_SERVER, PL_INFO), "UdpSerTimerOut -> %d", para->para1);

    XS32 nIndex = 0;
    if(NULL != para)
    {
        nIndex = para->para1;
        if(nIndex == UDP_MSG_SEND_TIMER)
        {
            UdpSend(g_udpSendMsg.ip, g_udpSendMsg.port, g_udpSendBuf, strlen(g_udpSendBuf), g_udpSendMsg.index);
        }
    }

    return 0;
}

void UdpStartSendTimer(void)
{
    t_PARA timerpara;
    t_BACKPARA backpara = {0};
    timerpara.fid = FID_UDP_SERVER;
    timerpara.len = 100;
    timerpara.mode = TIMER_TYPE_LOOP;
    timerpara.pre  = TIMER_PRE_HIGH;
    backpara.para1 = UDP_MSG_SEND_TIMER;
    
    if (XSUCC != XOS_TimerStart(&udpTestTimer, &timerpara, &backpara))
    {
        XOS_Trace(MD(FID_UDP_SERVER, PL_EXP), "ftp start test timer error!");
        return ;
    }
}


void UdpGetSendLenVar(XCHAR* pStr , XU32 *pOut)
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


void UdpSerCmd(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    char cmd[64];
    XU16 i = 0;
    XU32 nSendLen = 0;
    int cliNum = 0;
    
    if(siArgc < 2)
    {
        return;
    }
    
    XOS_StrNcpy(cmd, ppArgv[0], sizeof(cmd));
    XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "command:%s,args:%d.",cmd,siArgc);
    if(!XOS_StrCmp(cmd, "udpserver")) 
    {
        if(siArgc >= 4) 
        {
            XOS_MemCpy(g_localIp, ppArgv[1], sizeof(g_localIp));
            g_localPort = atoi(ppArgv[2]);
            XOS_MemCpy(g_PeerIp, ppArgv[3], sizeof(g_PeerIp));
            g_peerPort = atoi(ppArgv[4]);
            cliNum = atoi(ppArgv[5]);
            for(i = 0;i< cliNum;i++)
            {
                StartUdpSer();
                XOS_Sleep(100);
            }
        }
    } 
    else if(!XOS_StrCmp(cmd, "udpsend")) 
    {
        if(siArgc >= 6) 
        {
            UdpGetSendLenVar(ppArgv[3], &nSendLen);
            for(i = 0 ;i < nSendLen; i++)
            {
                g_udpSendBuf[i] = 'f';
            }
            g_udpSendBuf[i] = '\0';
            memcpy(g_udpSendMsg.ip, ppArgv[1], strlen(ppArgv[1]));
            g_udpSendMsg.port = atoi(ppArgv[2]);
            g_udpSendMsg.index = atoi(ppArgv[5]);

            //UdpStartSendTimer();

            for(int i = 0;i < atoi(ppArgv[4]); i++)
//            while(1)
            {
                UdpSend(ppArgv[1], atoi(ppArgv[2]), g_udpSendBuf,nSendLen, atoi(ppArgv[5]));
                //XOS_Sleep(3000);
            }
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "udpsend complete");
        }
        else
        {
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR:udpsend args error.");
        }
    }
    else if(!XOS_StrCmp(cmd, "udpclose")) 
    {
        UdpClose(atoi(ppArgv[1]));
        msgNum = 1;
    }
    else if(!XOS_StrCmp(cmd, "udprelease")) 
    {
        UdpLinkRelease(atoi(ppArgv[1]));
    }

}

void UdpSerShow(CLI_ENV* pCliEnv, XS32 siArgc, XCHAR** ppArgv)
{
    XS32 index = -1;
    XS32 i =0 ;
    XS32 j = 0;

    /*udp cfg list start */
    XOS_CliExtPrintf(pCliEnv,
        "udp server statistic list \r\n----------------------------\r\n");
    XOS_CliExtPrintf(pCliEnv,
        "%-6s%-12s%-6s%-12s\r\n",
        "index",
        "serveraddr",
        "listenport",
        "type"
        );

    XOS_MutexLock(&g_udpArray_Mutex);
    for(i=0; i<MaxObj; i++)
    {
        if(g_udpArray[i]._used)
        {
            XOS_CliExtPrintf(pCliEnv,
                "----------------------------\r\n");
            XOS_CliExtPrintf(pCliEnv,
                "%-6d%-12x%-6d   udp\r\n",
                i,g_udpArray[i]._StartAck.localAddr.ip,g_udpArray[i]._StartAck.localAddr.port
                );
            if(g_udpArray[i].curclients >0)
            {            
                for(j = 0; j<g_udpArray[i].curclients; j++ )
                {
                    
                    XOS_CliExtPrintf(pCliEnv,
                        "%-6d%-12x%-6d    client\r\n",
                        j,g_udpArray[i]._Conn[j].peerAddr.ip,g_udpArray[i]._Conn[j].peerAddr.port
                        );
                }
            }

        }
    }
    XOS_MutexUnlock(&g_udpArray_Mutex);

    XOS_CliExtPrintf(pCliEnv,
        "recvall = %lld, msgNum = %d, recvNum = %d", recvall, msgNum, recvNum );

    recvall = 0;

    /*end of list */
    XOS_CliExtPrintf(pCliEnv,
        "----------------------------\r\n");
    return ;
}

void StartUdpSer()
{
    t_LINKINIT linkInit;
    
    int index = 0;
    int i = 0;
    
    linkInit.linkType = eUDP;
    linkInit.ctrlFlag = eNullLinkCtrl;
    linkInit.appHandle = (HAPPUSER)g_appNum++;

    for(i = 0; i < MaxObj; i++)
    {
        if(!g_udpArray[i]._used)
        {
            g_udpArray[i]._app = linkInit.appHandle;
            g_udpArray[i]._used = 1;
            index = i;
            break;
        }
    }
    
    if(i == MaxObj)
    {
        printf("UdpServer Link number is enough, max=%d\n", MaxObj);
        return;
    }
    
    UdpLinkInit(linkInit);
}

void UdpSend(XS8* peerip, XS32 peerport, XS8* msg, XS32 len, XS32 index)
{
    t_DATAREQ dataReq;
    XS32 i = 0;

    XOS_StrtoIp(peerip,&(dataReq.dstAddr.ip));
    for(int j =0;j< 1;j++)
    {
        XS8* pMsg = (XS8*)XOS_MemMalloc(FID_UDP_SERVER, len);
        XOS_MemSet(pMsg, 0, len);
        dataReq.dstAddr.port = peerport + j;
        dataReq.linkHandle = g_udpArray[index + j]._IniAck.linkHandle;
        dataReq.msgLenth = len;
        dataReq.pData = pMsg;

        XOS_MemCpy(dataReq.pData, msg, len);
        XOS_CpsTrace(MD(FID_UDP_SERVER, PL_INFO), "SendData ip:%x,port:%d,len:%d\n",
            dataReq.dstAddr.ip,dataReq.dstAddr.port,len);

        UdpSendData((XS8 *)&dataReq, sizeof(t_DATAREQ));

    }
}

void UdpClose(XS32 index)
{
    if(index < 0 || index >= MaxObj )
    {
        return;
    }
    if(g_udpArray[index]._used == 0)
    {
        return;
    }
    t_LINKCLOSEREQ *closeReq = NULL;
    closeReq = (t_LINKCLOSEREQ *)XOS_MemMalloc(FID_UDP_SERVER, sizeof(t_LINKCLOSEREQ));
    XOS_MemSet(closeReq, 0, sizeof(t_LINKCLOSEREQ));

    closeReq->linkHandle = g_udpArray[index]._IniAck.linkHandle;
    closeReq->cliAddr.ip = g_udpArray[index]._Conn[0].peerAddr.ip;
    closeReq->cliAddr.port = g_udpArray[index]._Conn[0].peerAddr.port;

    XOS_CpsTrace(MD(FID_UDP_SERVER, PL_INFO), "UdpClose ip:%x,port:%d,index:%d,linkhandle:%x",
        g_udpArray[index]._Conn[0].peerAddr.ip,g_udpArray[index]._Conn[0].peerAddr.port,index,closeReq->linkHandle);

    g_udpArray[index]._used = 0;

    UdpLinkClose(*closeReq);
}

XS32 UdpLinkInit(t_LINKINIT & linkInit)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_UDP_SERVER, sizeof(t_LINKINIT));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_UDP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkInit;
        XOS_MemCpy(pMsg->message, &linkInit, sizeof(t_LINKINIT));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_UDP_SERVER, pMsg);
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR: UDP init send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 UdpLinkStart(t_LINKSTART& linkStart)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_UDP_SERVER, sizeof(t_LINKSTART));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_UDP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStart;
        XOS_MemCpy(pMsg->message, &linkStart, sizeof(t_LINKSTART));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_UDP_SERVER, pMsg);
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR: UDP start send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 UdpLinkClose(t_LINKCLOSEREQ& linkClose)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_UDP_SERVER, sizeof(t_LINKCLOSEREQ));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_UDP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eLinkStop;
        XOS_MemCpy(pMsg->message, &linkClose, sizeof(t_LINKCLOSEREQ));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_UDP_SERVER, pMsg);
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR: LinkClose send msg failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 UdpSendData(XS8* data, XS32 len)
{
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_UDP_SERVER, len);
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_UDP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eFastMsgPrio;
        pMsg->msgID = eSendData;
        XOS_MemCpy(pMsg->message, data, len);

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_UDP_SERVER, pMsg);
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR: UdpSendData send data failed.");
            return XERROR;
        }
        return XSUCC;
    }
    return ret;
}

XS32 UdpLinkRelease(XS32 index)
{
    t_LINKRELEASE linkRelease;
    t_XOSCOMMHEAD *pMsg = XNULL;
    XS8 ret = XERROR;

    linkRelease.linkHandle = g_udpArray[index]._IniAck.linkHandle;

    pMsg = (t_XOSCOMMHEAD*)XOS_MsgMemMalloc(FID_UDP_SERVER, sizeof(t_LINKRELEASE));
    if(pMsg) {
        pMsg->datasrc.PID = XOS_GetLocalPID();
        pMsg->datasrc.FID = FID_UDP_SERVER;
        pMsg->datadest.PID = XOS_GetLocalPID();
        pMsg->datadest.FID = FID_NTL;
        pMsg->prio = eNormalMsgPrio;
        pMsg->msgID = eLinkRelease;
        XOS_MemCpy(pMsg->message, &linkRelease, sizeof(t_LINKRELEASE));

        ret = XOS_MsgSend(pMsg);
        if(ret != XSUCC)
        {
            /*数据指示消息，应该先释放收到数据*/
            XOS_MsgMemFree(FID_UDP_SERVER, pMsg);
            XOS_Trace(FILI, FID_UDP_SERVER, PL_ERR, "ERROR: UdpLinkRelease failed.");
            return XERROR;
        }
        g_udpArray[index]._used = 0;
        return XSUCC;
    }
    return ret;
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

